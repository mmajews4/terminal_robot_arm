.PHONY: clean run
.SILENT:

terminal_robot_arm.o: terminal_robot_arm.c
	gcc -o $@ $< -lm

run: terminal_robot_arm.o
	./$<

clean:
	rm -rf terminal_robot_arm.o

