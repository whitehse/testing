-- Example for SF18
-- https://www.youtube.com/watch?v=biNdEqWoxrE

-- declare the protocol
sf18_proto = Proto("sf18", "SharkFest'18 Protocol (lua)")

-- declare the value strings
local vs_funcs = {
    [20] = "connect",
    [21] = "connect_ack",
    [40] = "request_data",
    [41] = "request_reply",
    [60] = "disconnect",
    [61] = "disconnect_ack"
}

local vs_dataid = {
    [0] = "read short",
    [1] = "read long",
    [2] = "read string"
}

-- declare the fields
local f_func = ProtoField.uint8("sf18.func", "Function", base.DEC, vs_funcs)
local f_len = ProtoField.uint16("sf18.len", "Length", base.DEC)
local f_id = ProtoField.uint32("sf18.id", "ID", base.DEC)
local f_dataid = ProtoField.uint8("sf18.data.id", "Data ID", base.DEC, vs_dataid)
local f_data_short = ProtoField.int16("sf18.data.short", "Data Short", base.DEC)
local f_data_long = ProtoField.int32("sf18.data.long", "Data Long", base.DEC)
local f_data_string = ProtoField.string("sf18.data.string", "Data String")

sf18_proto.fields = { f_func, f_len, f_id, f_dataid,
                      f_data_short, f_data_long, f_data_string }

-- create the dissection function
function sf18_proto.dissector(buffer, pinfo, tree)

    -- Set the protocol column
    pinfo.cols['protocol'] = "SF18"

    -- create the SF18 protocol tree item
    local t_sf18 = tree:add(sf18_proto, buffer())
    local offset = 0

    -- Add the header tree item and populate it
    local t_hdr = t_sf18:add(buffer(offset, 3), "Header")
    t_hdr:add(f_func, buffer(offset, 1))
    t_hdr:add(f_len, buffer(offset + 1, 2))
    local func_code = buffer(offset, 1):uint()
    offset = offset + 3

    -- Set the info column to the name of the function
    pinfo.cols['info'] = vs_funcs[func_code]

    --dissect common part for connect and disconnect functions
    if func_code == 20 or func_code == 21 or func_code == 60 or func_code == 61 then
        -- A connect or connect ack or disconnect or disconnect_ack
        t_sf18:add_le(f_id, buffer(offset, 4))
    end

    -- A request_data or request_reply message
    if func_code == 40 or func_code == 41 then
        -- dissect common part of request functions
        t_sf18:add(f_dataaid, buffer(offset, 1))

        -- dissect request_reply data body
        if func_code == 41 then
            -- A request_reply
            local dataid = buffer(offset, 1):uint()
            offset = offset + 1
            if dataid == 0 then
                -- a read short data item
                t_sf18:add_le(f_data_short, buffer(offset, 2))
            end
            if dataid == 1 then
                -- a read long data item
                t_sf18:add_le(f_data_long, buffer(offset, 4))
            end
            if dataid == 2 then
                -- a read string data item
                t_sf18:add(f_data_string, buffer(offset, 15))
            end
        end
    end
end

-- load the tcp port table
tcp_table = DissectorTable.get("tcp.port")
-- register the protocol to port 54321
tcp_table:add(54321, sf18_proto)

