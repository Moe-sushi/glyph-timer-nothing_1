all :
	cc gtimer.c -Wall -Wextra -O3 -z noexecstack -z now -fstack-protector-all -fPIE -static -ffunction-sections -fdata-sections -Wl,--gc-sections -o gtimer
install :all
	cp gtimer ${PREFIX}/bin/gtimer
	chmod 777 ${PREFIX}/bin/gtimer
clean :
	rm gtimer
format :
	clang-format -i gtimer.c
check :
	clang-tidy  --checks=-clang-analyzer-security.insecureAPI.strcpy,-clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling gtimer.c --