


#include <unistd.h>
#include <cerrno>
#include <cstring>
#include "Infra/System.h"
#include "Logger/Define.h"
extern "C" {
#include <sys/wait.h>
}

namespace {
using namespace ArcFace;

inline int32_t linuxSystem(const char* command) {
	if (command == nullptr) {
		return (1);
	}

	int32_t status = -1;
	pid_t pid = vfork();
	if (pid < 0) {
		errorf("vfork() failed>>>>\n");
		status = -1;
	} else if (pid == 0) {
        execl("/bin/sh", "sh", "-c", command, (char*)0);
		_exit(127);
	} else {
		while (waitpid(pid, &status, 0) < 0) {
			if (errno != EINTR) {
				status = -1;
				break;
			}
		}
	}

	return status;
}

} // namespace noname


namespace ArcFace {
namespace Infra {

int32_t systemCall(const char* command) {
    return linuxSystem(command);
}


}
}