#include "generated/game_strings.generated.cpp"

#define ID32_RAW(string) (IdString32(sizeof(#string)-1, #string))
#define ID64_RAW(string) (IdString64(sizeof(#string)-1, #string))

#define ID(id) (IdString32(_id32_##id))
#define ID32(id, string) (IdString32(_id32_##id))
#define ID64(id, string) (IdString64(_id64_##id))

#if DEVELOPMENT
	#define ID_STR(id) idstring_to_str(id)
	#define ID32_STR(id) idstring_to_str(IdString32(id))
	#define ID64_STR(id) idstring_to_str(IdString64(id))
#endif