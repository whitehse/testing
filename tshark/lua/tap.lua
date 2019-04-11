local packets = 0;

local tap = Listener.new("ip","ip.addr == 10.0.1.0/24")

function tap.reset()
    packets = 0;
end

function tap.packet(pinfo,tvb,ip)
    packets = packets + 1
end

function tap.draw()
    print("Packets to/from 10.0.1.0/24", packets)
end
