Custom Shell    										   2023.02 - 2023.03

•	Developed a feature-rich custom shell in C, featuring built-in commands, I/O redirection, pipe functionality and signal handling

•	Integrated built-in commands by ‘execve’ sys call, and ‘jobs’ command by employing an array-based job tracking system to manage background processes; utilized the ‘kill’ command with the ‘SIGCONT’ signal to resume suspended jobs for the ‘fg’ command

•	Incorporated four types of I/O redirections ‘>’, ‘>>’, ‘<’, ‘<<’ by utilizing the ‘open()’ command with varying parameters to acquire appropriate file descriptors and then redirecting the file descriptor to ‘stdin’ or ‘stdout’ using ‘dup2()’

•	Implemented the pipe functionality to combine commands by employing ‘pipe()’ to establish dedicated communication channels, ‘fork()’ to enable process parallel execution, ‘dup2()’ to direct output-to-input flow between processes for each command; used the ‘waitpid()’ system call to ensure proper synchronization and coordination among child processes

•	Implemented signal handling using ‘signal()’ system call, effectively capturing various signals like SIGINT and SIGSTP; employed customized signal handler functions to respond appropriately to different signals, enhancing user control and shell functionality
