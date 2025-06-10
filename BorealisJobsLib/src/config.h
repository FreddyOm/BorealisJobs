#ifdef BOREALIS_BUILD_DLL
#define BOREALIS_API __declspec(dllexport)
#else
#define BOREALIS_API __declspec(dllimport)
#endif // BOREALIS_BUILD_DLL

static constexpr int NUM_FIBERS()
{
	return 150;
}

static_assert(NUM_FIBERS() < 2028,
	"There can only be a maximum of 2028 fibers present at the same time!");