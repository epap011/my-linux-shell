# my-linux-shell
CS345 - Operating Systems | Assignment

This is a simple linux shell, which developed on a virtual machine (via QEMU Emulator) 

Supported Functions:
-Simple Commands (e.g ls, pwd, cd, exit, whoami, ...)
-Single Pipes (e.g echo “Hello world” | wc -w) 
-Multiple Pipes (e.g cat test.txt | sort | head -2)
-Redirections (e.g ls > files.txt, cat < data.txt, ls >> files.txt)

What i learned:
-The use of fork(2)
-The use of exec(3)
-The use of pipe(2)
-The use of wait(2) 
-The use of dup2(2)
-The use of chdir(2)

-Working remotly with QEMU (X11 Forwarding)
