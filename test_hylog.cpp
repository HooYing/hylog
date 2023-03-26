#include "hylog/AsyncLogging.h"
#include "hylog//Log.h"

off_t kRollSize = 500 * 1000 * 1000;

hying::AsyncLogging* g_asyncLog = NULL;


void asyncOutput(const char* msg, int len)
{
    g_asyncLog->append(msg, len);
}

class Test
{
public:
    void show()
    {
        int cnt = 0;
        for (int t = 0; t < 100; ++t)
        {
            LOG_INFO << "Hello world:" << cnt;
            ++cnt;
        }
        std::cout << __func__ << std::endl;

    }
};

void bench(bool longLog)
{
    hying::Logger::setOutput(asyncOutput);

    Test t;
    t.show();

    log_info("test:%s", "aaa");
}

int main(int argc, char** argv)
{
	char name[256] = { '\0' };
	strncpy(name, argv[0], sizeof name - 1);
	hying::AsyncLogging log("hello", kRollSize);
	log.start();
	g_asyncLog = &log;

	bool longLog = argc > 1;
	bench(longLog);
}

