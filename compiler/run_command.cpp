#if 1
// NOTE(kalle): If appname is given, it _needs_ to be the absolute path
// appname can also be given as the first string in cmd, and then the path will get searched
BOOL run_command(char *appname, char *cmd, char *cwd) {
	// SECURITY_ATTRIBUTES saAttr;

	// saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	// saAttr.bInheritHandle = TRUE;
	// saAttr.lpSecurityDescriptor = NULL;

	// HANDLE child_read_handle;
	// HANDLE child_write_handle;

	// if (!CreatePipe(&child_read_handle, &child_write_handle, &saAttr, 0)) {
	// 	fprintf(stderr, "Error in CreatePipe while generating `%s`\n\n`Windows error code: %ld`\n\n", cmd, GetLastError());
	// 	return false;
	// }

	// // We need to make sure to kill cmd.exe and any child processes it might create (for e.g. cl.exe)
	// // We do this with "job objects", https://blogs.msdn.microsoft.com/oldnewthing/20131209-00/?p=2433
	// HANDLE job_object = CreateJobObject(nullptr, nullptr);

	// JOBOBJECT_EXTENDED_LIMIT_INFORMATION info = { };
	// info.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
	// SetInformationJobObject(job_object, JobObjectExtendedLimitInformation, &info, sizeof(info));

	STARTUPINFO si;
	memset(&si, 0, sizeof(si));
	// si.dwFlags |= STARTF_USESTDHANDLES;
	si.cb = sizeof(si);
	PROCESS_INFORMATION pi;
	memset(&pi, 0, sizeof(pi));

	BOOL success = CreateProcessA(appname, cmd, 0, 0, FALSE, 0, 0, cwd, &si, &pi);
	if (!success) {
		fprintf(stderr, "Error in CreateProcess while generating `%s`\n\n`Windows error code: %ld`\n\n", cmd, GetLastError());
		return false;
	}
	WaitForSingleObject(pi.hProcess, INFINITE);

	DWORD exit_code = 0;
	success = GetExitCodeProcess(pi.hProcess, &exit_code);
	if (!success) {
		fprintf(stderr, "Error in GetExitCodeProcess while generating `%s`\n\n`Windows error code: %ld`\n\n", cmd, GetLastError());
	// } else {
		// success = exit_code == 0;
		// if (!success) {
		// 	fprintf(stderr, "Error while generating `%s`\n\n%s\n", cmd, child_output_buffer);
		// } else {
		// 	fprintf(stdout, "`%s`:\n%s\n", cmd, child_output_buffer);
		// }
	}

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	return success;
}
#else
BOOL run_command(char *appname, char *cmd, char *cwd) {
	SECURITY_ATTRIBUTES saAttr;

	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	saAttr.bInheritHandle = TRUE;
	saAttr.lpSecurityDescriptor = NULL;

	HANDLE child_read_handle;
	HANDLE child_write_handle;

	if (!CreatePipe(&child_read_handle, &child_write_handle, &saAttr, 0)) {
		fprintf(stderr, "Error in CreatePipe while generating `%s`\n\n`Windows error code: %ld`\n\n", cmd, GetLastError());
		return false;
	}

	// We need to make sure to kill cmd.exe and any child processes it might create (for e.g. cl.exe)
	// We do this with "job objects", https://blogs.msdn.microsoft.com/oldnewthing/20131209-00/?p=2433
	HANDLE job_object = CreateJobObject(nullptr, nullptr);

	JOBOBJECT_EXTENDED_LIMIT_INFORMATION info = { };
	info.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
	SetInformationJobObject(job_object, JobObjectExtendedLimitInformation, &info, sizeof(info));

	STARTUPINFO si;
	memset(&si, 0, sizeof(si));
	si.dwFlags |= STARTF_USESTDHANDLES;
	si.hStdOutput = child_write_handle;
	si.hStdError = child_write_handle;
	si.cb = sizeof(si);
	PROCESS_INFORMATION pi;
	memset(&pi, 0, sizeof(pi));

	Stopwatch sw;
	sw.start();
	BOOL success = CreateProcessA(appname, cmd, 0, 0, TRUE, CREATE_DEFAULT_ERROR_MODE | CREATE_NO_WINDOW, 0, cwd, &si, &pi);
	if (!success) {
		fprintf(stderr, "Error in CreateProcess while generating `%s`\n\n`Windows error code: %ld`\n\n", cmd, GetLastError());
		return false;
	}

	// Assign the process to the above job
	success = AssignProcessToJobObject(job_object, pi.hProcess);

	CloseHandle(child_write_handle);

	static const size_t bufsize = 8 * MB;
	MemoryArena arena = {};
	char *child_output_buffer = PUSH_STRING(arena, bufsize);
	unsigned cursor = 0;
	while (true) {
		DWORD bytes_read;
		// Sleep(30);
		char *buffer = child_output_buffer + cursor;
		success = ReadFile(child_read_handle, buffer, bufsize - cursor, &bytes_read, NULL);
		if (!success || bytes_read == 0)
			break;

		cursor += bytes_read;
	}

	CloseHandle(child_read_handle);

	for (unsigned i = 0; i < cursor; i++) {
		if (child_output_buffer[i] == '\r') {
			child_output_buffer[i] = ' ';
		}
	}
	child_output_buffer[cursor] = '\0';

	DWORD exit_code = 0;
	success = GetExitCodeProcess(pi.hProcess, &exit_code);
	if (!success) {
		fprintf(stderr, "Error in GetExitCodeProcess while generating `%s`\n\n`Windows error code: %ld`\n\n", cmd, GetLastError());
	} else {
		success = exit_code == 0;
		if (!success) {
			fprintf(stderr, "Error while generating `%s`\n\n%s\n", cmd, child_output_buffer);
		} else {
			fprintf(stdout, "`%s`:\n%s\n", cmd, child_output_buffer);
		}
	}

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	free_memory(arena);
	return success;
}
#endif