/* src/rpc-handler/server-rpc-builder.x */

struct entry {
    string username<256>;
    string operation<256>;
    string filename<256>;
    string datetime<256>;
};


program RPC_SERVICE_DATETIME{
        version RPC_SERVICE {
        int print_datetime(entry) = 1;
        }=1;
}=100495794;