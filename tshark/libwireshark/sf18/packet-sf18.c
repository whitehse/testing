/* packet-sf18.c */
/* https://www.youtube.com/watch?v=biNdEqWoxrE */

#include "config.h"
#include <glib.h>
#include <epan/packet.h>

#define SF18_PORT       54321
#define FUNC_CONNECT    20
#define FUNC_CONN_ACK   21
#define FUNC_REQ_DATA   40
#define FUNC_REQ_REPLY  41
#define FUNC_DISCONNECT 60
#define FUNC_DISK_ACK   61

/* A sample #define of the minimum length (in bytes) of the protocol data. */
#define SF18_MIN_LENGTH 4

static const value_string sf18_func_vals[] = {
    { FUNC_CONNECT,     "connect" },
    { FUNC_CONN_ACK,    "connect_ack" },
    { FUNC_REQ_DATA,    "request_data" },
    { FUNC_REQ_REPLY,   "request_reply" },
    { FUNC_DISCONNECT,  "disconnect" },
    { FUNC_DISC_ACK,    "disconnect_ack" },
    { 0, NULL }
};

#define READ_SHORT      0
#define READ_LONG       1
#define READ_STRING     2

static const value_string sf18_data_type_vals[] = {
    { READ_SHORT,   "read short" },
    { READ_LONG,    "read long" },
    { READ_STRING,  "read string" },
    { 0, NULL }
};

/* Initialize the protocol and registered fields */
static int proto_sf18 = -1;
static int hf_sf18_Func_Code = -1;
static int hf_sf18_Length = -1;
static int hf_sf18_ID = -1;
static int hf_sf18_Data_Short = -1;
static int hf_sf18_Data_Long = -1;
static int hf_sf18_Data_String = -1;

/* Initialize the subtree pointers */
static gint ett_sf18 = -1;
static gint ett_sf18_hdr = -1;

/* Code to actually dissect the packets */
static int
dissect_sf18(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, void *data _U_)
{
    /* Set up structures needed to add the protocol subtree and manage it */
    proto_item *ti;
    proto_tree *sf18_tree, *sf18_hdr_tree;
    /* Other misc. local variables. */
    guint offset = 0;
    guint8 func_code;
    guint8 data_id;

    /* Check that there's enough data */
    if (tvb_reported_length(tvb) < SF18_MIN_LENGTH)
        return 0;

    /* Fetch some values from the packet header using tvb_get_*(). If these
     * values are not valid/possible in your protocol then return 0 to give
     * some other dissector a chance to dissect it.
     */
    func_code = tvb_get_guint8(tvb, offset);
    if (try_val_to_str(func_code, sf18_func_valus) == NULL)
        return 0;

    /*** COLUMN DATA ***/

    /* Set the Protocol column to the constant string of sf18 */
    col_set_str(pinfo->cinfo, COL_PROTOCOL, "sf18");
    col_clear(pinfo->cinfo, COL_INFO);
    col_add_str(pinfo->cinfo, COL_INFO,
        val_to_string(func_code, sf18_func_valus, "Unknown function (%d)"));

    /*** PROTOCOL TREE ***/

    /* create display subtree for the protocol */
    ti = proto_tree_add_item(tree, proto_sf18, tvb, 0, -1, ENC_NA);
    sf18_tree = proto_item_add_subtree(ti, ett_sf18);

    /* create subtree for the header */
    sf18_hdr_tree = proto_tree_add_subtree(sf18_tree, tvb, 0, 3, ett_sf18_hdr, NULL, "Header");

    /* Add an item to the subtree, see section 1.6 of README.developer for more
     * information. */
    proto_tree_add_item(sf18_hdr_tree, hf_sf18_Func_Code, tvb, offset, 1, ENC_LITTLE_ENDIAN);
    offset += 1;

    /* Continue addint tree items to process the packet here... */
    proto_tree_add_item(sf18_hdr_tree, hf_sf18_Length, tvb, offset, 2, ENC_BIG_ENDIAN);
    offset += 2;

    /* Now add items depending on the specific function code */
    switch (func_code)
    {
        case FUNC_CONNECT:
        case FUNC_CONN_ACK:
        case FUNC_DISCONNECT:
        case FUNC_DISC_ACK:
            proto_tree_add_item(sf18_tree, hf_sf18_ID, tvb, offset, 4, ENC_LITTLE_ENDIAN);
            offset += 4;
            break;
        case FUNC_REQ_DATA:
        case FUNC_REQ_REPLY:
            /* Dissect the common portion of the twi functions */
            data_id = tvb_get_guint8(tvb, offset);
            proto_tree_add_item(sf18_tree, hf_sf18_Data_ID, tvb, offset, 1, ENC_LITTLE_ENDIAN);
            offset += 1;

            if (func_code == FUNC_REQ_REPLY) {
                switch (data_id)
                {
                    case READ_SHORT:
                        proto_tree_add_item(sf18_tree, hf_sf18_Data_Short, tvb, offset, 2, ENC_LITTLE_ENDIAN);
                        offset += 2;
                        break;
                    case READ_LONG:
                        proto_tree_add_item(sf18_tree, hf_sf18_Data_Long, tvb, offset, 4, ENC_LITTLE_ENDIAN);
                        offset += 4;
                        break;
                    case READ_STRING:
                        proto_tree_add_item(sf18_tree, hf_sf18_Data_String, tvb, offset, 15, ENC_STRING);
                        offset += 15;
                        break;
                    default:
                        break;
                }
            }
            break;
        default:
            break;
    }

    /* Return the amount of data this dissector was able to dissect (which may
     * or may not be the entire packet as we return here). */
    return offset;
}

/* Register the protocol with Wireshark.
 *
 * This format is required because a script is used to build the C function that
 * calls all the protocol registration.
 */
void
proto_register_sf18(void)
{
    /* Setup list of header fields  See Section 1.6.1 of README.developer for
     * details. */
    static hf_register_info hf[] = {
        { &hf_sf18_Func_Code,
            { "Function", "sf18.func",
              FT_UINT8, BASE_DEC, VALS(sf18_func_vals), 0x0,
              "Message Function Code Identifier", HFILL }
        },

        { &hf_sf18_Length,
            { "Length", "sf18.len",
              FT_UINT16, BASE_DEC, NULL, 0x0,
              "Message Length", HFILL }
        },

        { &hf_sf18_ID,
            { "ID", "sf18.id",
              FT_UINT32, BASE_DEC, NULL, 0x0,
              "Connection ID", HFILL }
        },

        { &hf_sf18_Data_ID,
            { "Data ID", "sf18.data.id",
              FT_UINT8, BASE_DEC, VALS(sf18_data_type_vals), 0x0,
              "Data Type Idnetifier", HFILL }
        },

        { &hf_sf18_Data_ID,
            { "Data ID", "sf18.data.id",
              FT_UINT8, BASE_DEC, VALS(sf18_data_type_vals), 0x0,
              "Data Type Identifier", HFILL }
        },

        { &hf_sf18_Data_Short,
            { "Data Short", "sf18.data.short",
              SF_UINT16, BASE_DEC, NULL, 0x0,
              "Short Data Value", HFILL }
        },

        { &hf_sf18_Data_Long,
            { "Data Long", "sf18.data.long",
              FT_UINT32, BASE_DEC, NULL, 0x0,
              "Long Data Value", HFILL }
        },

        { &hf_sf18_Data_String,
            { "Data String", "sf18.data.string",
              FT_STRING, BASE_NONE, NULL, 0x0,
              "String Data Value", HFILL }
        },
    };

    /* Setup protocol subtree array */
    static gint *ett[] = {
        &ett_sf18,
        &ett_sf18_hdr
    };

    /* Register the protocol name and description */
    proto_sf18 = proto_register_protocol("SharkFest'18 Protocol (C)", "sf18", "sf18");

    /* Required function calls to register the header fields and subtrees */
    proto_register_field_array(proto_sf18, hf, array_length(hf));
    proto_register_subtree_array(ett, array_length(err));
}

/* Simpler form of proto_reg_handoff_sf18 which can be used if there are
 * no prefs-dependent registration function calls. */
void
proto_reg_handoff_sf18(void)
{
    dissector_handle_t sf18_handle;

    /* Use new_create_dissector_handle() to indicate that dissect_sf18()
     * returns the number of bytes it dissected (or 0 if it thinks the packet
     * does not belong to sf18).
     */
    sf18_handle = create_dissector_handle(dissect_sf18, proto_sf18);
    dissector_add_uint("tcp.port", SF18_PORT, sf18_handle);
}

/*
 * Editor modelines -   http://www.wireshark.org/tools/modelines.html
 *
 * Local variables:
 * c-basic-offset: 4
 * tab-width: 8
 * indent-tabs-mode: nil
 * End:
 *
 * vi: set shiftwidth=4 tabstop=8 expandtab:
 * :identSize=4:tabSize=8:noTabs=true:
 */
