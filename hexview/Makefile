OBJDIR := objbin64
GCC_OBJ_CMD := gcc -g -O -c
GCC_LNK_CMD := gcc -o hexview

hexview: control.o file.o main.o tokenizer.o util.o
	$(GCC_LNK_CMD) $(OBJDIR)/control.o $(OBJDIR)/file.o $(OBJDIR)/main.o $(OBJDIR)/tokenizer.o $(OBJDIR)/util.o

control.o:
	mkdir -p $(OBJDIR)
	$(GCC_OBJ_CMD) -o $(OBJDIR)/control.o control.c

file.o:
	mkdir -p $(OBJDIR)
	$(GCC_OBJ_CMD) -o $(OBJDIR)/file.o file.c

main.o:
	mkdir -p $(OBJDIR)
	$(GCC_OBJ_CMD) -o $(OBJDIR)/main.o main.c

tokenizer.o:
	mkdir -p $(OBJDIR)
	$(GCC_OBJ_CMD) -o $(OBJDIR)/tokenizer.o tokenizer.c

util.o:
	mkdir -p $(OBJDIR)
	$(GCC_OBJ_CMD) -o $(OBJDIR)/util.o util.c