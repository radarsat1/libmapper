
#include <cstring>
#include <iostream>
#include <arpa/inet.h>
#include <cstdio>
#include <cstdlib>
#include <array>

#include <mapper/mapper_cpp.h>

#ifdef WIN32
#define usleep(x) Sleep(x/1000)
#else
#include <unistd.h>
#endif

int received = 0;

void insig_handler(mapper_signal sig, int instance_id, const void *value,
                   int count, mapper_timetag_t *timetag)
{
    if (value) {
        printf("--> destination got %s", mapper_signal_name(sig));
        int len = mapper_signal_length(sig);
        switch (mapper_signal_type(sig)) {
            case 'i': {
                int *v = (int*)value;
                for (int i = 0; i < len; i++) {
                    printf(" %d", v[i]);
                }
                break;
            }
            case 'f': {
                float *v = (float*)value;
                for (int i = 0; i < len; i++) {
                    printf(" %f", v[i]);
                }
            }
            case 'd': {
                double *v = (double*)value;
                for (int i = 0; i < len; i++) {
                    printf(" %f", v[i]);
                }
            }
            default:
                break;
        }
        printf("\n");
    }
    received++;
}

int main(int argc, char ** argv)
{
    int i = 0, result = 0;

    mapper::Device dev("mydevice");

    mapper::Signal sig = dev.add_input("in1", 1, 'f', "meters", 0,
                                       0, insig_handler, 0);
    dev.remove_input(sig);
    dev.add_input("in2", 2, 'i', 0, 0, 0, insig_handler, 0);
    dev.add_input("in3", 2, 'i', 0, 0, 0, insig_handler, 0);
    dev.add_input("in4", 2, 'i', 0, 0, 0, insig_handler, 0);

    sig = dev.add_output("out1", 1, 'f', "na", 0, 0);
    dev.remove_output(sig);
    sig = dev.add_output("out2", 3, 'd', "meters", 0, 0);

    while (!dev.ready()) {
        dev.poll(100);
    }

    std::cout << "device " << dev.name() << " ready..." << std::endl;
    std::cout << "  ordinal: " << dev.ordinal() << std::endl;
    std::cout << "  id: " << dev.id() << std::endl;
    std::cout << "  interface: " << dev.interface() << std::endl;
    const struct in_addr* a = dev.ip4();
    if (a)
        std::cout << "  host: " << inet_ntoa(*a) << std::endl;
    std::cout << "  port: " << dev.port() << std::endl;
    std::cout << "  num_fds: " << dev.num_fds() << std::endl;
    std::cout << "  num_inputs: " << dev.num_inputs() << std::endl;
    std::cout << "  num_outputs: " << dev.num_outputs() << std::endl;
    std::cout << "  num_incoming_maps: " << dev.num_maps(DI_INCOMING) << std::endl;
    std::cout << "  num_outgoing_maps: " << dev.num_maps(DI_OUTGOING) << std::endl;

    // access properties through the property getter
    std::cout << "name: " << (const char*)dev.property("name") << std::endl;

    int value[] = {1,2,3,4,5,6};
    dev.set_property("foo", 6, value);
    const int *tempi = dev.property("foo");
    std::cout << "foo: ";
    for (i = 0; i < 6; i++)
        std::cout << tempi[i] << " ";
    std::cout << std::endl;

    // test std::array<std::string>
    std::cout << "set and get std::array<std::string>: ";
    std::array<std::string, 3> a1 = {{"one", "two", "three"}};
    dev.set_property("foo", a1);
    const std::array<std::string, 8> a2 = dev.property("foo");
    for (i = 0; i < 8; i++)
        std::cout << a2[i] << " ";
    std::cout << std::endl;

    // test std::array<const char*>
    std::cout << "set and get std::array<const char*>: ";
    std::array<const char*, 3> a3 = {{"four", "five", "six"}};
    dev.set_property("foo", a3);
    std::array<const char*, 3> a4 = dev.property("foo");
    for (i = 0; i < a4.size(); i++)
        std::cout << a4[i] << " ";
    std::cout << std::endl;

    // test plain array of const char*
    std::cout << "set and get const char*[]: ";
    const char* a5[3] = {"seven", "eight", "nine"};
    dev.set_property("foo", 3, a5);
    const char **a6 = dev.property("foo");
    std::cout << a6[0] << " " << a6[1] << " " << a6[2] << std::endl;

    // test plain array of float
    std::cout << "set and get float[]: ";
    float a7[3] = {7.7f, 8.8f, 9.9f};
    dev.set_property("foo", 3, a7);
    const float *a8 = dev.property("foo");
    std::cout << a8[0] << " " << a8[1] << " " << a8[2] << std::endl;

    // test std::vector<const char*>
    std::cout << "set and get std::vector<const char*>: ";
    const char *a9[3] = {"ten", "eleven", "twelve"};
    std::vector<const char*> v1(a9, std::end(a9));
    dev.set_property("foo", v1);
    std::vector<const char*> v2 = dev.property("foo");
    std::cout << "foo: ";
    for (std::vector<const char*>::iterator it = v2.begin(); it != v2.end(); ++it)
        std::cout << *it << " ";
    std::cout << std::endl;

    // test std::vector<std::string>
    std::cout << "set and get std::vector<std::string>: ";
    const char *a10[3] = {"thirteen", "14", "15"};
    std::vector<std::string> v3(a10, std::end(a10));
    dev.set_property("foo", v3);
    std::vector<std::string> v4 = dev.property("foo");
    std::cout << "foo: ";
    for (std::vector<std::string>::iterator it = v4.begin(); it != v4.end(); ++it)
        std::cout << *it << " ";
    std::cout << std::endl;

    mapper::Property p("temp", "tempstring");
    dev.set_property(p);
    std::cout << p.name << ": " << (const char*)p << std::endl;

    dev.remove_property("foo");
    std::cout << "foo: " << dev.property("foo").value
        << " (should be 0x0)" << std::endl;

    std::cout << "signal: " << (const char*)sig << std::endl;

    mapper::Signal::Iterator iter = dev.inputs().begin();
    for (; iter != iter.end(); ++iter) {
        std::cout << "input: " << (const char*)(*iter) << std::endl;
    }

    mapper::Admin adm(SUBSCRIBE_ALL);
    mapper::Map map = adm.map(dev.outputs()[0], dev.inputs()[1]);
    map.set_mode(MO_EXPRESSION).set_expression("y=x[0:1]+123");
    double d[3] = {1., 2., 3.};
    map.source().set_minimum(mapper::Property(0, 3, d));
    adm.update(map);

    while (dev.num_maps() <= 0) {
        dev.poll(100);
    }

    std::vector <double> v(3);
    while (i++ < 100) {
        dev.poll(10);
        adm.poll();
        v[i%3] = i;
        sig.update(v);
    }

    // try combining queries
    mapper::Device::Iterator r = adm.db().devices_by_name_match("my");
    r += adm.db().devices_by_property(mapper::Property("num_inputs", 4),
                                      QUERY_GREATER_THAN_OR_EQUAL);
//    mapper::Device::Iterator r = q1 + q2;
    for (; r != r.end(); r++) {
        std::cout << "  r device: " << (const char*)(*r) << std::endl;
    }

    // check db records
    std::cout << "db records:" << std::endl;
    for (auto const &device : adm.db().devices()) {
        std::cout << "  device: " << (const char*)device.property("name") << std::endl;
        for (auto const &signal : device.inputs()) {
            std::cout << "  input signal: " << device.name()
                      << "/" << signal.name() << std::endl;
        }
        for (auto const &signal : device.outputs()) {
            std::cout << "  output signal: " << device.name()
                      << "/" << signal.name() << std::endl;
        }
    }
    for (auto const &m : adm.db().maps()) {
        std::cout << "  map: ";
        if (m.num_sources() > 1)
            std::cout << "[";
        for (int i = 0; i < m.num_sources(); i++) {
            std::cout << m.source(i).device().name()
                      << "/" << m.source(i).signal().name() << ", ";
        }
        std::cout << "\b\b";
        if (m.num_sources() > 1)
            std::cout << "]";
        std::cout << " -> " << m.destination().device().name()
                  << "/" << m.destination().signal().name() << std::endl;
    }

    printf("Test %s.\n", result ? "FAILED" : "PASSED");
    return result;
}
