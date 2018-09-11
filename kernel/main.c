
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            main.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include "type.h"
#include "stdio.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "fs.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "proto.h"

#include "2048Game.h" // For 2048 game
#include "Guess.h"
#include "cal.h"
#include "queen.h"
//#include "chess.h"

/*======================================================================*
                            kernel_main
 *======================================================================*/
PUBLIC int kernel_main() 
{
    struct task* p_task;
    struct proc* p_proc= proc_table;
    char* p_task_stack = task_stack + STACK_SIZE_TOTAL;
    u16   selector_ldt = SELECTOR_LDT_FIRST;
    u8    privilege;
    u8    rpl;
    int   eflags;
    int   i, j;
    int   prio;

    // start the process
    for (i = 0; i < NR_TASKS+NR_PROCS; i++) {
        if (i < NR_TASKS) {
            /* task */
            p_task    = task_table + i;
            privilege = PRIVILEGE_TASK;
            rpl       = RPL_TASK;
            eflags    = 0x1202; /* IF=1, IOPL=1, bit 2 is always 1   1 0010 0000 0010(2)*/
            prio      = 15;     //set the priority to 15
        }
        else {
            /* user process */
            p_task    = user_proc_table + (i - NR_TASKS);
            privilege = PRIVILEGE_USER;
            rpl       = RPL_USER;
            eflags    = 0x202; /* IF=1, bit 2 is always 1              0010 0000 0010(2)*/
            prio      = 5;     //set the priority to 5
        }

        strcpy(p_proc->name, p_task->name); /* set prio name */
        p_proc->pid = i;            /* set pid */

        p_proc->run_count = 0;

        p_proc->ldt_sel = selector_ldt;

        memcpy(&p_proc->ldts[0], &gdt[SELECTOR_KERNEL_CS >> 3],
               sizeof(struct descriptor));
        p_proc->ldts[0].attr1 = DA_C | privilege << 5;
        memcpy(&p_proc->ldts[1], &gdt[SELECTOR_KERNEL_DS >> 3],
               sizeof(struct descriptor));
        p_proc->ldts[1].attr1 = DA_DRW | privilege << 5;
        p_proc->regs.cs = (0 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
        p_proc->regs.ds = (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
        p_proc->regs.es = (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
        p_proc->regs.fs = (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
        p_proc->regs.ss = (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
        p_proc->regs.gs = (SELECTOR_KERNEL_GS & SA_RPL_MASK) | rpl;

        p_proc->regs.eip = (u32)p_task->initial_eip;
        p_proc->regs.esp = (u32)p_task_stack;
        p_proc->regs.eflags = eflags;

        p_proc->p_flags = 0;
        p_proc->p_msg = 0;
        p_proc->p_recvfrom = NO_TASK;
        p_proc->p_sendto = NO_TASK;
        p_proc->has_int_msg = 0;
        p_proc->q_sending = 0;
        p_proc->next_sending = 0;

        for (j = 0; j < NR_FILES; j++) {
            p_proc->filp[j] = 0;
        }

        p_proc->ticks = p_proc->priority = prio;
        p_proc->run_state = 1;
        p_task_stack -= p_task->stacksize;
        p_proc++;
        p_task++;
        selector_ldt += 1 << 3;
    }

    proc_table[4].run_state = 0;
    proc_table[5].run_state = 0;

    // init process
    k_reenter = 0;
    ticks = 0;

    p_proc_ready = proc_table;

    init_clock();
    init_keyboard();

    restart();
}


/*****************************************************************************
 *                                get_ticks
 *****************************************************************************/
PUBLIC int get_ticks() {
    MESSAGE msg;
    reset_msg(&msg);
    msg.type = GET_TICKS;
    send_recv(BOTH, TASK_SYS, &msg);
    return msg.RETVAL;
}

PUBLIC void addTwoString(char *to_str, char *from_str1, char *from_str2) {
    int i = 0, j = 0;
    while (from_str1[i] != 0) {
        to_str[j++]=from_str1[i++];
    }
    i = 0;
    while (from_str2[i] != 0) {
        to_str[j++]=from_str2[i++];
    }
    to_str[j]=0;
}

void shell(char *tty_name) {
    int fd;
    //int isLogin = 0;
    char rdbuf[512];
    char cmd[512];
    char arg1[512];
    char arg2[512];
    char buf[1024];
    char temp[512];

    int fd_stdin  = open(tty_name, O_RDWR);
    assert(fd_stdin  == 0);
    int fd_stdout = open(tty_name, O_RDWR);
    assert(fd_stdout == 1);
    // The boot animation
    // animation();
    char current_dirr[512] = "/";
    while (1) {
        // clear the array ！
        clearArr(rdbuf, 512);
        clearArr(cmd, 512);
        clearArr(arg1, 512);
        clearArr(arg2, 512);
        clearArr(buf, 1024);
        clearArr(temp, 512);
        
        //help();
        printf("root@owls%s:~$ ", current_dirr);
        int r = read(fd_stdin, rdbuf, 512);
        if (strcmp(rdbuf, "") == 0) {
            continue;
        }
        int i = 0;
        int j = 0;
        while ((rdbuf[i] != ' ') && (rdbuf[i] != 0)) {
            cmd[i] = rdbuf[i];
            i++;
        }
        i++;
        while ((rdbuf[i] != ' ') && (rdbuf[i] != 0)) {
            arg1[j] = rdbuf[i];
            i++;
            j++;
        }
        i++;
        j = 0;
        while ((rdbuf[i] != ' ') && (rdbuf[i] != 0)) {
            arg2[j] = rdbuf[i];
            i++;
            j++;
        }
        // clear
        rdbuf[r] = 0;

        // Command "procmng"
        if (strcmp(cmd, "procmng") == 0) {
            ProcessManage();
        }

        // Command "help"
        else if (strcmp(cmd, "help") == 0) {
            help();
        }      

        // Command "clear"
        else if (strcmp(cmd, "clear") == 0) {
            clear(); 
        }

        // Command "ls"
        else if (strcmp(cmd, "ls") == 0) {
            ls(current_dirr);
        }

        // Command "about"
        else if (strcmp(cmd, "about") == 0) {
            printAbout();
        }

        // Command "pause 4"
        else if (strcmp(rdbuf, "pause 4") == 0) {
            proc_table[4].run_state = 0 ;
            ProcessManage();
        }

        // Commandd "pause5"
        else if (strcmp(rdbuf, "pause 5") == 0) {
            proc_table[5].run_state = 0 ;
            ProcessManage();
        }

        // Command "pause6"
        else if (strcmp(rdbuf, "pause 6") == 0) {
            proc_table[6].run_state = 0 ;
            ProcessManage();
        }

        // Command "kill 4"
        else if (strcmp(rdbuf, "kill 4") == 0) {
        //            proc_table[4].p_flags = 1;
        //            ProcessManage();
            printf("cannott kill this process!");
        }

        // Command "kill 5"
        else if (strcmp(rdbuf, "kill 5") == 0) {
            proc_table[5].p_flags = 1;
            ProcessManage();
        }

        // Command "kill 6"
        else if (strcmp(rdbuf, "kill 6") == 0) {
            proc_table[6].p_flags = 1;
            ProcessManage();
        }

        // Command "resume 4"
        else if (strcmp(rdbuf, "resume 4") == 0 ) {
            proc_table[4].run_state = 1 ;
            ProcessManage();
        }

        // Command "resume 5"
        else if (strcmp(rdbuf, "resume 5") == 0 ) {
            proc_table[5].run_state = 1 ;
            ProcessManage();
        }

        // Command "resume 6"
        else if (strcmp(rdbuf, "resume 6") == 0 ) {
            proc_table[6].run_state = 1 ;
            ProcessManage();
        }

        // Command "up 4"
        else if (strcmp(rdbuf, "up 4") == 0 ) {
            proc_table[4].priority = proc_table[4].priority*2;
            ProcessManage();
        }

        // Command "up 5"
        else if (strcmp(rdbuf, "up 5") == 0 ) {
            proc_table[5].priority = proc_table[5].priority*2;
            ProcessManage();
        }

        // Command "up 6"
        else if (strcmp(rdbuf, "up 6") == 0 ) {
            proc_table[6].priority = proc_table[6].priority*2;
            ProcessManage();
        }

        // Command "touch"
        else if (strcmp(cmd, "new") == 0) {
            if (arg1[0] != '/') {
                addTwoString(temp,current_dirr, arg1);
                memcpy(arg1, temp, 512);                
            }

            fd = open(arg1, O_CREAT | O_RDWR);
            if (fd == -1) {
                printf("Failed to create file! Please check the filename!\n");
                continue ;
            }
            write(fd, buf, 1);
            printf("File created: %s (fd %d)\n", arg1, fd);
            close(fd);
        }

        // Command "cat"
        else if (strcmp(cmd, "cat") == 0) {
            if (arg1[0] != '/') {
                addTwoString(temp,current_dirr, arg1);
                memcpy(arg1, temp, 512);
            }

            fd = open(arg1, O_RDWR);
            if (fd == -1) {
                printf("Failed to open file! Please check the filename!\n");
                continue ;
            }
            if (!verifyFilePass(arg1, fd_stdin)) {
                printf("Authorization failed\n");
                continue;
            }
            read(fd, buf, 1024);
            close(fd);
            printf("%s\n", buf);
        }

        // Command "vi"
        else if (strcmp(cmd, "vi") == 0) {
            if (arg1[0] == 0) {
                printf("Failed to open file! Please input the filename!\n");
                continue;
            }
            if (arg1[0] != '/') {
                addTwoString(temp, current_dirr, arg1);
                memcpy(arg1, temp, 512);
            }

            fd = open(arg1, O_RDWR);
            if (fd == -1) {
                printf("Please check the filename!\n");
                continue ;
            }
            if (!verifyFilePass(arg1, fd_stdin)) {
                printf("Authorization failed\n");
                continue;
            }
            int tail = read(fd_stdin, rdbuf, 512);
            rdbuf[tail] = 0;

            write(fd, rdbuf, tail + 1);
            close(fd);
        }

        // Command "de"
        else if (strcmp(cmd, "rm") == 0) {

            if (arg1[0] != '/') {
                addTwoString(temp, current_dirr, arg1);
                memcpy(arg1, temp, 512);
            }

            int result;
            result = unlink(arg1);
            if (result == 0) {
                printf("File deleted!\n");
                continue;
            }
            else {
                printf("Failed to delete file! Please check the filename!\n");
                continue;
            }
        }

        // Command "cp"
        else if (strcmp(cmd, "cp") == 0) {
            //get the content of file
            if (arg1[0] != '/') {
                addTwoString(temp, current_dirr, arg1);
                memcpy(arg1, temp, 512);
            }
            fd = open(arg1, O_RDWR);
            if (fd == -1) {
                printf("File not exists! Please check the filename!\n");
                continue ;
            }
            int tail = read(fd, buf, 1024);
            close(fd);

            if (arg2[0] != '/') {
                addTwoString(temp, current_dirr, arg2);
                memcpy(arg2, temp, 512);
            }
            /*create the file*/
            fd = open(arg2, O_CREAT | O_RDWR);
            if (fd == -1) {
                //file exist
            }
            else {
                //file not exist or create file
                char temp2[1024];
                temp2[0] = 0;
                write(fd, temp2, 1);
                close(fd);
            }
            //set file to value
            fd = open(arg2, O_RDWR);
            write(fd, buf, tail + 1);
            close(fd);
        }

        // Command "mv"
        else if (strcmp(cmd, "mv") == 0) {
             if (arg1[0] != '/') {
                addTwoString(temp, current_dirr, arg1);
                memcpy(arg1, temp, 512);
            }
            //get the content of file first
            fd = open(arg1, O_RDWR);
            if (fd == -1) {
                printf("File not exists! Please check the filename!\n");
                continue ;
            }
           
            int tail = read(fd, buf, 1024);
            close(fd);

            if (arg2[0] != '/') {
                addTwoString(temp, current_dirr, arg2);
                memcpy(arg2, temp, 512);
            }
            /*create the file*/
            fd = open(arg2, O_CREAT | O_RDWR);
            if (fd == -1) {
                //file exist
            }
            else{
                //file not exist or create file
                char temp2[1024];
                temp2[0] = 0;
                write(fd, temp2, 1);
                close(fd);
            }
            // set file to value
            fd = open(arg2, O_RDWR);
            write(fd, buf, tail+1);
            close(fd);
            // delete the file
            unlink(arg1);
        }

        // Command "encrypt"
        else if (strcmp(cmd, "encrypt") == 0) {
            fd = open(arg1, O_RDWR);
            if (fd == -1) {
                printf("File not exists! Please check the filename!\n");
                continue ;
            }
            if (!verifyFilePass(arg1, fd_stdin)) {
                printf("Authorization failed\n");
                continue;
            }
            doEncrypt(arg1, fd_stdin);
        }

        // Command "test"
        else if (strcmp(cmd, "test") == 0) {
            doTest(arg1);
        }

        // Command "game"
        else if (strcmp(cmd, "game") == 0) {
        	game(fd_stdin);
        }

        // Command "mkdir"
        else if (strcmp(cmd, "mkdir") == 0) {
            i = j = 0;
            while (current_dirr[i] != 0) {
                arg2[j++] = current_dirr[i++];
            }
            i = 0;
            
            while (arg1[i] != 0) {
                arg2[j++] = arg1[i++];
            }
            arg2[j] = 0;
            printf("%s\n", arg2);
            fd = mkdir(arg2);
        }

        // Command "cd"
        else if (strcmp(cmd, "cd") == 0) {
            if (strcmp(arg1, "..") == 0) {
                memcpy(arg2, current_dirr, 512);
                j = 0;
                int k = 0;
                int count = 0;
                while (arg2[k] != 0) {
                    if (arg2[k++] == '/') {
                        count++;
                    }
                }
                int index = 0;
                for (i = 0; arg2[i] != 0; i++) {
                    if (arg2[i] == '/') {
                        index++;
                    }
                    if (index < count - 1) {
                        arg1[j++] = arg2[i];
                    }
                }
                arg1[j++] = '/';
                arg1[j] = 0;
            }
            else if (arg1[0] == 0) {
                arg1[0] = '/';
                arg1[1] = 0;
            }
            else { // not absolute path from root
                i = j = 0;
                while (current_dirr[i] != 0) {
                    arg2[j++] = current_dirr[i++];
                }
                i = 0;
                while (arg1[i] != 0) {
                    arg2[j++] = arg1[i++];
                }
                arg2[j++] = '/';
                arg2[j] = 0;
                memcpy(arg1, arg2, 512);
            }
            //printf("%s\n", arg1);
            fd = open(arg1, O_RDWR);

            if (fd == -1) {
                printf("The path not exists! Please check the pathname!\n");
            }
            else {
                memcpy(current_dirr, arg1, 512);
                //printf("Change dir %s success!\n", current_dirr);
            }
        }

        // Command "pwd"
        else if(strcmp(cmd, "pwd") == 0) {
            printf("%s\n",current_dirr);
        }

        // Command "uname"
        else if(strcmp(cmd,"uname") == 0) {
            printf("Owl'S\n");
        }

        // Command "snake"
        else if (strcmp(cmd, "snake") == 0) {
            snakeGame();
        }

        // Commmand "2048"
        else if (strcmp(cmd, "2048") == 0) {
        	start2048Game(fd_stdin, fd_stdout);
        }
 
       // Commmand "Num"
        else if (strcmp(cmd, "Guess") == 0) {
        	startGuessGame(fd_stdin, fd_stdout);
        }
       // Commmand "cal"
        else if (strcmp(cmd, "Calculator") == 0) {
        	startcal(fd_stdin, fd_stdout);
        }
         // Commmand "cal"
        else if (strcmp(cmd, "Queen") == 0) {
        	startqueen(fd_stdin, fd_stdout);
        }
 // Commmand "chess"
      //  else if (strcmp(cmd, "chess") == 0) {
      //  	startchess(fd_stdin, fd_stdout);
     //   }

        // Command not supported
        else {
            printf("Command not supported!\n");
        }
    }
}

/*======================================================================*
                               TestA
 *======================================================================*/

//A process
void TestA(){
    animation();
    shell("/dev_tty0");
    
    assert(0);
}

/*======================================================================*
                               TestB
 *======================================================================
*/
//B process
void TestB(){
	shell("/dev_tty1");
	assert(0); /* never arrive here */
}
/*======================================================================*
                               TestC
 *======================================================================*/
//C process
void TestC(){
	// shell("/dev_tty2");
	assert(0);
}

/*****************************************************************************
 *                                panic
 *****************************************************************************/
PUBLIC void panic(const char *fmt, ...) {
    int i;
    char buf[256];
    /* 4 is the size of fmt in the stack */
    va_list arg = (va_list) ((char*) &fmt + 4);
    i = vsprintf(buf, fmt, arg);
    printl("%c !!panic!! %s", MAG_CH_PANIC, buf);
    /* should never arrive here */
    __asm__ __volatile__("ud2");
}

/*****************************************************************************
 *                                Custom Command
 *****************************************************************************/
char* findpass(char *src) {
    char pass[128];
    int flag = 0;
    char *p1, *p2;

    p1 = src;
    p2 = pass;

    while (p1 && (*p1 != ' ')) {
        if (*p1 == ':') {
            flag = 1;
        }

        if (flag && (*p1 != ':')) {
            *p2 = *p1;
            p2++;
        }
        p1++;
    }
    *p2 = '\0';

    return pass;
}

void clearArr(char *arr, int length) {
    int i;
    for (i = 0; i < length; i++) {
        arr[i] = 0;
    }
}

void printAbout() {
    clear(); 
    if (current_console == 0) {
        printf("================================================================================");
        printf("                                  O-Smile-master                                         ");
        printf("                     Authors:                                                   ");
        printf("                              Hanqiu Wang     1652704                             ");
        printf("                              Minghui Yang    1652703                            ");
        printf("================================================================================");
    }
    else {
    	printf("[TTY #%d]\n", current_console);
    }
}

void clear() {	
    clear_screen(0, console_table[current_console].cursor);
    console_table[current_console].crtc_start = console_table[current_console].orig;
    console_table[current_console].cursor = console_table[current_console].orig;
}

void doTest(char *path) {
    struct dir_entry *pde = find_entry(path);
    printl(pde->name);
    printl("\n");
    printl(pde->pass);
    printl("\n");
}

int verifyFilePass(char *path, int fd_stdin) {
    char pass[128];

    struct dir_entry *pde = find_entry(path);

    /*printl(pde->pass);*/

    if (strcmp(pde->pass, "") == 0) {
        return 1;
    }

    printl("Please input the file password: ");
    read(fd_stdin, pass, 128);

    if (strcmp(pde->pass, pass) == 0) {
        return 1;
    }

    return 0;
}

void doEncrypt(char *path, int fd_stdin) {
    //search the file
    /*struct dir_entry *pde = find_entry(path);*/

    char pass[128] = { 0 };

    printl("Please input the new file password: ");
    read(fd_stdin, pass, 128);

    if (strcmp(pass, "") == 0) {
        /*printl("A blank password!\n");*/
        strcpy(pass, "");
    }
    //encrypt the file
    int i, j;

    char filename[MAX_PATH];
    memset(filename, 0, MAX_FILENAME_LEN);
    struct inode * dir_inode;

    if (strip_path(filename, path, &dir_inode) != 0) {
        return 0;
    }

    /* path: "/" */
    if (filename[0] == 0) {
        return dir_inode->i_num;
    }
    /**
     * Search the dir for the file.
     */
    int dir_blk0_nr = dir_inode->i_start_sect;
    int nr_dir_blks = (dir_inode->i_size + SECTOR_SIZE - 1) / SECTOR_SIZE;
    int nr_dir_entries = dir_inode->i_size / DIR_ENTRY_SIZE;
    int m = 0;
    struct dir_entry * pde;
    for (i = 0; i < nr_dir_blks; i++) {
        RD_SECT(dir_inode->i_dev, dir_blk0_nr + i);
        pde = (struct dir_entry *) fsbuf;
        for (j = 0; j < SECTOR_SIZE / DIR_ENTRY_SIZE; j++,pde++) {
            if (memcmp(filename, pde->name, MAX_FILENAME_LEN) == 0) {
                // delete the file
                strcpy(pde->pass, pass);
                WR_SECT(dir_inode->i_dev, dir_blk0_nr + i);
                return;
                /*return pde->inode_nr;*/
            }
            if (++m > nr_dir_entries) {
                break;
            }
        }

         /* all entries have been iterated */
        if (m > nr_dir_entries) {
            break;
        }
    }
}

void help() {
    printf("================================================================================");
    printf("                                  O-Smile                                         ");
    printf("                                                                                ");
    printf("         A microkernel OS developed by Hanqiu WANG, Minghui YANG in 2018      ");
    printf("================================================================================");
    printf("    clear                         : clear the screen                            ");
    printf("    ls                            : list files in current directory             ");
    printf("    new         [filename]        : create a new file                           ");
   // printf("    cat         [filename]        : display content of the file                 ");
   // printf("    vi          [filename]        : modify the content of the file              ");
    printf("    de          [filename]        : delete a file                               ");
   // printf("    cp          [source] [dest]   : copy a file                                 ");
   // printf("    mv          [source] [dest]   : move a file                                 ");
    //printf("    encrypt     [filename]        : encrypt a file                              ");
    printf("    cd          [pathname]        : change the directory                        ");
    printf("    mkdir       [dirname]         : create a new directory                      ");
                   
     printf("    2048                          : start the 2048 game\n "
                        );
                      
     printf("   Guess                         : start the guess number game \n"
                         );

 printf("    Queen                         : start the guess number game \n"
                         );

 printf("    Calculator                    : start the guess number game\n" 
                         );



//printf("    procmng                          : process management                          ");
   // printf("    uname                         : display the system's name                   ");
    printf("================================================================================");
}

void ProcessManage() {
    int i;
    printf("=============================================================================\n");
    printf("         pID      |    name       |      priority    |     running        \n");
    printf("-----------------------------------------------------------------------------\n");
    for (i = 0 ; i < NR_TASKS + NR_PROCS ; ++i) {
        printf("          %2d          %10s              %2d              %d\n",
               proc_table[i].pid, proc_table[i].name, proc_table[i].priority, proc_table[i].run_state);
    }
    printf("=============================================================================\n");
    printf("=          Usage: pause  [pid]  you can pause one process                   =\n");
    printf("=          	      resume [pid]  you can resume one process                  =\n");
    printf("=                 kill   [pid]  kill the process                            =\n");
    printf("=                 up     [pid]  improve the process priority                =\n");
    printf("=============================================================================\n");
}


/*======================================================================*
                            The boot animation
 *======================================================================*/
void p0(int n, int color){
    for(int i = 0; i < n; i++){

        disp_color_str("0",color);
    }
}

void animation(){

    int c0=0x70;int c2=0x72;int c3 = 0x73;int c6=0x76;int c7 = 0x77;int c8 = 0x78;int cb=0x7B; int ce=0x7E; int cf=0x7F;
    clear();
    milli_delay(1000); 
    p0(14,c7);p0(2,c8);p0(25,c7);p0(2,c8);p0(37,c7);//p0(2,c8);//p0(14,c8);p0(11,c7);
    milli_delay(1000);
     p0(12,c7);p0(2,c8);p0(2,c7);p0(2,c8);p0(21,c7);p0(2,c8);p0(2,c7);p0(2,c8);p0(35,c7);//p0(53,c7);p0(2,c8);p0(14,c3);p0(2,c8);p0(9,c7);
    milli_delay(1000);
     p0(10,c7);p0(2,c8);p0(6,c7);p0(2,c8);p0(17,c7);p0(2,c8);p0(6,c7);p0(2,c8);p0(33,c7);// p0(48,c7);//p0(2,c8);p0(3,c8);p0(16,c3);p0(2,c8);p0(1,c8);p0(8,c7);
    milli_delay(1000);
     p0(8,c7);p0(2,c8);p0(10,c7);p0(2,c8);p0(13,c7);p0(2,c8);p0(10,c7);p0(2,c8);p0(31,c7);// p0(38,c7);//p0(1,c8);p0(2,c3);p0(1,c8);p0(5,c7);p0(3,c8);p0(20,c3);p0(1,c8);p0(9,c7);
    milli_delay(1000);
    p0(80,c7);
    milli_delay(1000);
    p0(80,c7);
    milli_delay(1000);
    p0(80,c7);
    milli_delay(1000);
    p0(80,c7);
    milli_delay(1000);
    p0(80,c7);
    milli_delay(1000);
    p0(80,c7);
    milli_delay(1000);
    p0(80,c7);
    milli_delay(1000);
    p0(80,c7);
    milli_delay(1000);
    p0(80,c7);
    milli_delay(1000);
    p0(18,c7);p0(2,c8);p0(18,c7);p0(2,c8);p0(40,c7);
    milli_delay(1000);
    p0(20,c7);p0(2,c8);p0(14,c7);p0(2,c8);p0(42,c7);
    milli_delay(1000);
    p0(22,c7);p0(2,c8);p0(10,c7);p0(2,c8);p0(44,c7);
    milli_delay(1000);
    p0(28,c7);p0(1,c8);p0(51,c7);
    milli_delay(1000);
    p0(80,c7);
    milli_delay(1000);
    p0(80,c7);
    milli_delay(1000);
    p0(80,c7);
    p0(80,c7);
    milli_delay(1000);
    p0(80,c7);
    milli_delay(1000);
    p0(80,c7);
  
   /* p0(11,c7);p0(12,c8);p0(14,c7);p0(1,c8);p0(3,c3);p0(1,c8);p0(3,c7);p0(2,c8);p0(23,c3);p0(2,c8);p0(8,c7);  
    milli_delay(1000);
    p0(9,c7);p0(2,c8);p0(11,c3);p0(2,c8);p0(13,c7);p0(1,c8);p0(2,c3);p0(1,c8);p0(3,c7);p0(2,c8);p0(23,c3);p0(2,c8);p0(9,c7);
    milli_delay(1000);
  p0(1,c8);p0(7,cb);p0(1,c8);p0(3,c7);p0(1,c8);p0(6,c6);p0(1,c8);p0(4,c7);p0(2,c8);p0(10,cb); p0(2,c8);p0(4,c7);p0(1,c8);p0(5,c2);p0(1,c8);p0(11,c7);
    milli_delay(1000);
    p0(13,c7);p0(1,c8);p0(4,c2);p0(1,c8);p0(13,c7);p0(1,c8);p0(6,c6);p0(1,c8);p0(7,c2);p0(4,c8);p0(18,c7);
    milli_delay(1000);
    p0(24,c7);p0(7,c8);p0(5,c7);p0(1,c8);p0(8,c7);p0(5,c8);p0(2,c7);p0(8,c8);p0(21,c7);*/
    milli_delay(10000);
    clear_screen(console_table[0].orig,console_table[0].con_size);
}

