#ifndef __MAPPER_DB_H__
#define __MAPPER_DB_H__

#ifdef __cplusplus
extern "C" {
#endif

/*! \file This file defines structs used to return information from
 *  the network database. */

#include <lo/lo.h>

/*! Bit flags for coordinating metadata subscriptions. Subsets of device
 *  information must also include SUB_DEVICE. */
#define MAPPER_SUBSCRIBE_NONE           0x00
#define MAPPER_SUBSCRIBE_DEVICES        0x01
#define MAPPER_SUBSCRIBE_INPUTS         0x02
#define MAPPER_SUBSCRIBE_OUTPUTS        0x04
#define MAPPER_SUBSCRIBE_SIGNALS        0x06 /* (MAPPER_SUBSCRIBE_INPUTS
                                                | MAPPER_SUBSCRIBE_OUTPUTS) */
#define MAPPER_SUBSCRIBE_INCOMING_MAPS  0x10
#define MAPPER_SUBSCRIBE_OUTGOING_MAPS  0x20
#define MAPPER_SUBSCRIBE_MAPS           0x30 /* ( MAPPER_SUBSCRIBE_INCOMING_MAPS
                                                | MAPPER_SUBSCRIBE_OUTGOING_MAPS) */
#define MAPPER_SUBSCRIBE_ALL            0xFF

/* Value for map status when it is active */
#define MAPPER_ACTIVE   0x1F

/*! A 64-bit data structure containing an NTP-compatible time tag, as
 *  used by OSC. */
typedef lo_timetag mapper_timetag_t;

/*! Possible operations for composing db queries. */
typedef enum {
    MAPPER_OP_UNDEFINED = -1,
    MAPPER_OP_DOES_NOT_EXIST,
    MAPPER_OP_EQUAL,
    MAPPER_OP_EXISTS,
    MAPPER_OP_GREATER_THAN,
    MAPPER_OP_GREATER_THAN_OR_EQUAL,
    MAPPER_OP_LESS_THAN,
    MAPPER_OP_LESS_THAN_OR_EQUAL,
    MAPPER_OP_NOT_EQUAL,
    NUM_MAPPER_OPS
} mapper_op;

/*! Describes what happens when the range boundaries are exceeded.
 *  @ingroup mapdb */
typedef enum {
    MAPPER_UNDEFINED,
    MAPPER_NONE,    //!< Value is passed through unchanged. This is the default.
    MAPPER_MUTE,    //!< Value is muted.
    MAPPER_CLAMP,   //!< Value is limited to the boundary.
    MAPPER_FOLD,    //!< Value continues in opposite direction.
    MAPPER_WRAP,    //!< Value appears as modulus offset at the opposite boundary.
    NUM_MAPPER_BOUNDARY_ACTIONS
} mapper_boundary_action;

/*! Describes the map modes.
 *  @ingroup mapdb */
typedef enum {
    MAPPER_MODE_UNDEFINED,  //!< Not yet defined
    MAPPER_MODE_RAW,        //!< No type coercion
    MAPPER_MODE_LINEAR,     //!< Linear scaling
    MAPPER_MODE_EXPRESSION, //!< Expression
    NUM_MAPPER_MODES
} mapper_mode;

typedef enum {
    MAPPER_SOURCE = 1,
    MAPPER_DESTINATION
} mapper_location;

/*! The set of possible directions for a signal or mapping slot. */
typedef enum {
    MAPPER_DIR_ANY  = 0x00,
    MAPPER_INCOMING = 0x01,
    MAPPER_OUTGOING = 0x02,
} mapper_direction;

/*! The set of possible actions on an instance, used to register callbacks
 *  to inform them of what is happening. */
#define MAPPER_NEW_INSTANCE         0x01 //!< New instance has been created.
#define MAPPER_UPSTREAM_RELEASE     0x02 //!< Instance was released upstream.
#define MAPPER_DOWNSTREAM_RELEASE   0x04 //!< Instance was released downstream.
#define MAPPER_INSTANCE_OVERFLOW    0x08 //!< No local instances left.
#define MAPPER_INSTANCE_ALL         0x0F

/*! Describes the voice-stealing mode for instances.
 *  @ingroup mapdb */
typedef enum {
    MAPPER_NO_STEALING,
    MAPPER_STEAL_OLDEST,    //!< Steal the oldest instance
    MAPPER_STEAL_NEWEST,    //!< Steal the newest instance
} mapper_instance_allocation_type;

/*! The set of possible actions on a database record, used to inform callbacks
 *  of what is happening to a record. */
typedef enum {
    MAPPER_ADDED,
    MAPPER_MODIFIED,
    MAPPER_REMOVED,
    MAPPER_EXPIRED,
} mapper_record_action;

#ifdef __cplusplus
}
#endif

#endif // __MAPPER_DB_H__
