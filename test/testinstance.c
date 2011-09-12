#include "../src/mapper_internal.h"
#include <mapper/mapper.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <lo/lo.h>

#include <unistd.h>
#include <signal.h>

#ifdef WIN32
#define usleep(x) Sleep(x/1000)
#endif

int automate = 1;

mapper_device source = 0;
mapper_device destination = 0;
mapper_signal sendsig = 0;
mapper_signal recvsig = 0;
mapper_signal_instance sendinst[5] = {0, 0, 0, 0, 0};
mapper_signal_instance recvinst[5] = {0, 0, 0, 0, 0};

int port = 9000;

int sent = 0;
int received = 0;
int done = 0;

void new_instance_handler(mapper_signal_instance si, mapper_db_signal props,
                          mapper_timetag_t *timetag, void *v);
void instance_handler(mapper_signal_instance si, mapper_db_signal props,
                      mapper_timetag_t *timetag, void *v);

/*! Creation of a local source. */
int setup_source()
{
    source = mdev_new("testsend", port, 0);
    if (!source)
        goto error;
    printf("source created.\n");

    float mn=0, mx=10;

    sendsig = mdev_add_output(source, "/outsig", 1, 'f', 0, &mn, &mx);
    // reserve an appropriate number of instances
    msig_reserve_instances(sendsig, 5, 0, 0);

    printf("Output signal registered.\n");
    printf("Number of outputs: %d\n", mdev_num_outputs(source));

    return 0;

  error:
    return 1;
}

void cleanup_source()
{
    if (source) {
        if (source->routers) {
            printf("Removing router.. ");
            fflush(stdout);
            mdev_remove_router(source, source->routers);
            printf("ok\n");
        }
        printf("Freeing source.. ");
        fflush(stdout);
        mdev_free(source);
        printf("ok\n");
    }
}

void instance_handler(mapper_signal_instance si, mapper_db_signal props,
                      mapper_timetag_t *timetag, void *v)
{
    if (v) {
        printf("--> destination %s instance %i got %f\n",
               props->name, si->id, (*(float*)v));
        received++;
    }
    else
        printf("--> destination %s instance %i got NULL\n",
               props->name, si->id);
}

void new_instance_handler(mapper_signal_instance si, mapper_db_signal props,
                          mapper_timetag_t *timetag, void *v)
{
    printf("--> destination %s got new instance %i\n",
           props->name, si->id);
    si->handler = instance_handler;
    instance_handler(si, props, timetag, v);
}

/*! Creation of a local destination. */
int setup_destination()
{
    destination = mdev_new("testrecv", port, 0);
    if (!destination)
        goto error;
    printf("destination created.\n");

    float mn=0, mx=1;
        
    recvsig = mdev_add_input(destination, "/insig", 1, 
                             'f', 0, &mn, &mx, 0, 0);
    msig_reserve_instances(recvsig, 5, new_instance_handler, 0);

    printf("Input signal registered.\n");
    printf("Number of inputs: %d\n", mdev_num_inputs(destination));

    return 0;

  error:
    return 1;
}

void cleanup_destination()
{
    if (destination) {
        printf("Freeing destination.. ");
        fflush(stdout);
        mdev_free(destination);
        printf("ok\n");
    }
}

void wait_local_devices()
{
    while (!(mdev_ready(source) && mdev_ready(destination))) {
        mdev_poll(source, 0);
        mdev_poll(destination, 0);

        usleep(50 * 1000);
    }
}

void loop()
{
    printf("-------------------- GO ! --------------------\n");
    int i = 0, j = 0;
    float value = 0;

    if (automate) {
        char source_name[1024], destination_name[1024];

        printf("%s\n", mdev_name(source));
        printf("%s\n", mdev_name(destination));

        lo_address a = lo_address_new_from_url("osc.udp://224.0.1.3:7570");
        lo_address_set_ttl(a, 1);

        lo_send(a, "/link", "ss", mdev_name(source), mdev_name(destination));

        msig_full_name(sendsig, source_name, 1024);
        msig_full_name(recvsig, destination_name, 1024);

        lo_send(a, "/connect", "ss", source_name, destination_name);

        lo_address_free(a);
    }

    while (i >= 0 && !done) {
        // here we should create, update and destroy some instances
        switch (rand() % 5) {
            case 0:
                // try to create a new instance
                for (j = 0; j < 5; j++) {
                    if (!sendinst[j]) {
                        sendinst[j] = msig_resume_instance(sendsig);
                        if (sendinst[j])
                            printf("--> Created new sender instance: %i\n", sendinst[j]->id);
                        break;
                    }
                }
                break;
            case 1:
                // try to destroy an instance
                j = rand() % 5;
                if (sendinst[j]) {
                    printf("--> Retiring sender instance %i\n", sendinst[j]->id);
                    msig_suspend_instance(sendinst[j]);
                    sendinst[j] = 0;
                    break;
                }
                break;
            default:
                j = rand() % 5;
                if (sendinst[j]) {
                    // try to update an instance
                    value = (rand() % 10) * 1.0f;
                    msig_update_instance(sendinst[j], &value);
                    printf("--> sender instance %i updated to %f\n", sendinst[j]->id, value);
                    sent++;
                }
                break;
        }
        mdev_poll(destination, 100);
        mdev_poll(source, 0);
        i++;
        usleep(100 * 1000);
    }
}

void ctrlc(int sig)
{
    done = 1;
}

int main()
{
    int result = 0;

    signal(SIGINT, ctrlc);

    if (setup_destination()) {
        printf("Error initializing destination.\n");
        result = 1;
        goto done;
    }

    if (setup_source()) {
        printf("Done initializing source.\n");
        result = 1;
        goto done;
    }

    wait_local_devices();

    loop();

    if (sent != received) {
        printf("Not all sent messages were received.\n");
        printf("Updated value %d time%s, but received %d of them.\n",
               sent, sent == 1 ? "" : "s", received);
        result = 1;
    }

  done:
    cleanup_destination();
    cleanup_source();
    printf("Test %s.\n", result ? "FAILED" : "PASSED");
    return result;
}
