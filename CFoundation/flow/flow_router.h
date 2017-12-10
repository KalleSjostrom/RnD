typedef char *flow_string;

#include "generated/flow_messages.generated.cpp"

#define FLOW_SET(type) (__global_last_arguments->filled_mask & ArgumentType::type)
#define FLOW_WRITE_RETURNS(...) write_return_values(*__global_last_message, __VA_ARGS__)
#define FLOW_STORE_OUT_EVENT(storage) { \
	FlowMessage &message = *__global_last_message; \
	storage = message.flow_out_event; \
}

