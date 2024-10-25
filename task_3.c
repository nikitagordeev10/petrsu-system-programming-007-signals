/*
 * task_3.c — Задача 10.6 из учебника. 
 * Напишите программу, с помощью которой можно было бы проверить функции синхронизации родительского и дочернего процессов из листинга 10.17. 
 * Процесс должен создавать файл и записывать в него число 0. Затем вызывается функция fork(), после чего родительский и дочерний процессы должны по очереди увеличивать число, прочитанное из файла. 
 * При каждом увеличении счетчика процесс должен выводить информацию о том, кто произвел увеличение - родитель или потомок.
 * 
 * Copyright (c) <2022> <Гордеев Никита>
 * 
 * This code is licensed under a MIT-style license.
 */


#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>

// Процедуры для синхронизации родительского и дочернего процессов
// =========================================================================
void err_sys(const char *x) {
    perror(x);
    exit(1);
}

static volatile sig_atomic_t sigflag;   // устанавливается обработчиком в ненулевое значение
static sigset_t newmask, oldmask, zeromask;

// единый обработчик для сигналов SIGUSR1 и SIGUSR2
static void sig_usr(int signo) {                              
    sigflag = 1;
}

void TELL_WAIT(void) {
    if (signal(SIGUSR1, sig_usr) == SIG_ERR)
        err_sys("ошибка вызова функции signal(SIGUSR1)");
    if (signal(SIGUSR2, sig_usr) == SIG_ERR)
        err_sys("ошибка вызова функции signal(SIGUSR2)");
    sigemptyset(&zeromask);
    sigemptyset(&newmask);
    sigaddset(&newmask, SIGUSR1);
    sigaddset(&newmask, SIGUSR2);

    // заблокировать сигналы SIGUSR1 и SIGUSR2, и сохранить текущую маску сигналов
    if (sigprocmask(SIG_BLOCK, &newmask, &oldmask) < 0)
        err_sys("SIG_BLOCK error");
}

// сообщить родительскому процессу, что мы готовы
void TELL_PARENT(pid_t pid) {
    kill(pid, SIGUSR2);
}

void WAIT_PARENT(void) {
    // ждать ответа от родительского процесса
    while (sigflag == 0)
        sigsuspend(&zeromask);
    sigflag = 0;

    //  Восстановить маску сигналов в начальное состояние.
    if (sigprocmask(SIG_SETMASK, &oldmask, NULL) < 0)
        err_sys("SIG_SETMASK error");
}

// сообщить дочернему процессу, что мы готовы
void TELL_CHILD(pid_t pid) {
    kill(pid, SIGUSR1);
}

void WAIT_CHILD(void) {
    // дождаться ответа от дочернего процесса 
    while (sigflag == 0)
        sigsuspend(&zeromask);
    sigflag = 0;

    // Восстановить маску сигналов в начальное состояние.
    if (sigprocmask(SIG_SETMASK, &oldmask, NULL) < 0)
        err_sys("SIG_SETMASK error");
}
// =========================================================================

int main() {

    int low = 0;
    int high = 15;

    FILE *f;
    pid_t pid;
    char filename[] = "task_3_record.txt";

    // открывает файл с режимом создания пустого файла для записи
    if ((f = fopen(filename, "w")) == NULL) {
        printf("Ошибка при вызове функции fopen\n");
        exit(EXIT_FAILURE);
    }

    // устанавливает внутренний указатель положения в файле на начало
    if (fseek(f, 0, SEEK_SET) == -1) {
        printf("Ошибка вызова функции fseek\n");
        exit(EXIT_FAILURE);
    }

    // запысывает стартовое значение счётчика в файл
    fprintf(f, "%d", low);

    // закрывает и разъединяет файл, связанный с потоком
    if (fclose(f) != 0) {
        printf("Ошибка при вызове функции fclose\n");
        exit(EXIT_FAILURE);
    }

    TELL_WAIT();

    // Создаем дочерний процесс
    if ((pid = fork()) < 0) {
        printf("Ошибка при вызове функции fork\n");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // процесс потомок
        while (low < high) {
            // открывает файл с режимом для чтения
            if ((f = fopen(filename, "r")) == NULL) {
                printf("Ошибка при вызове функции fopen\n");
                exit(EXIT_FAILURE);
            }

            // читаем текущее значение счётчика из файла 
            if (fscanf(f, "%d", &low));

            // проверка, что пора остановиться
            if (low >= high) break;

            // закрываем и разъединяем файл, связанный с потоком
            if (fclose(f) != 0) {
                printf("Ошибка при вызове функции fclose\n");
                exit(EXIT_FAILURE);
            }

            // увеличиваем счётчик
            low++;
            printf("Увеличил потомок, текущее значение = %d\n", low);
            
            // записываем новое значение счётчика
            if ((f = fopen(filename, "w")) == NULL) {
                printf("Ошибка при вызове функции fopen\n");
                exit(EXIT_FAILURE);
            }

            // записываем новое значение счётчика в файл
            fprintf(f, "%d", low);

            // закрываем и разъединяем файл, связанный с потоком
            if (fclose(f) != 0) {
                printf("Ошибка при вызове функции fclose\n");
                exit(EXIT_FAILURE);
            }
            
            TELL_PARENT(getppid());
            WAIT_PARENT();
        }
    } else {
        // процесс родитель
        WAIT_CHILD();
        while (low < high) {
            // открывает файл с режимом для чтения
            if ((f = fopen(filename, "r")) == NULL) {
                printf("Ошибка при вызове функции fopen\n");
                exit(EXIT_FAILURE);
            }

            // читаем текущее значение счётчика из файла 
            if (fscanf(f, "%d", &low));

            // проверка, что пора остановиться
            if (low >= high) break;

            // закрываем и разъединяем файл, связанный с потоком
            if (fclose(f) != 0) {
                printf("Ошибка при вызове функции fclose\n");
                exit(EXIT_FAILURE);
            }

            // увеличиваем счётчик
            low++;
            printf("Увеличил родитель, текущее значение = %d\n", low);
            
            // записываем новое значение счётчика
            if ((f = fopen(filename, "w")) == NULL) {
                printf("Ошибка при вызове функции fopen\n");
                exit(EXIT_FAILURE);
            }

            // записываем новое значение счётчика в файл
            fprintf(f, "%d", low);
            
            // закрываем и разъединяем файл, связанный с потоком
            if (fclose(f) != 0) {
                printf("Ошибка при вызове функции fclose\n");
                exit(EXIT_FAILURE);
            }

            TELL_CHILD(pid);
            WAIT_CHILD();
        }
    }
    return 0;
}

/* 
 * Материалы:
 * Глава 5. Стандартная библиотека ввода/вывода // «UNIX. Профессиональное программирование. 3-е изд.» Авторы: У. Ричард Стивенс, Стивен А. Раго Год: 2018 URL: https://www.rulit.me/data/programs/resources/pdf/UNIX-Professionalnoe-programmirovanie_RuLit_Me_609965.pdf (дата обращения: 03.11.2022).
 * Глава 5.10. Позиционирование в потоке // «UNIX. Профессиональное программирование. 3-е изд.» Авторы: У. Ричард Стивенс, Стивен А. Раго Год: 2018 URL: https://www.rulit.me/data/programs/resources/pdf/UNIX-Professionalnoe-programmirovanie_RuLit_Me_609965.pdf (дата обращения: 03.11.2022).
 * Листинг 10.17. Процедуры для синхронизации родительского и дочернего процессов // «UNIX. Профессиональное программирование. 3-е изд.» Авторы: У. Ричард Стивенс, Стивен А. Раго Год: 2018 URL: https://www.rulit.me/data/programs/resources/pdf/UNIX-Professionalnoe-programmirovanie_RuLit_Me_609965.pdf (дата обращения: 03.11.2022).
 * Функция fscanf // cpp.com URL: https://cpp.com.ru/shildt_spr_po_c/13/fscanf.html (дата обращения: 03.11.2022).
 * How does sig_atomic_t actually work? // stackoverflow URL: https://stackoverflow.com/questions/24931456/how-does-sig-atomic-t-actually-work (дата обращения: 03.11.2022).
 * warning: ignoring return value of ‘fscanf’ in C // stackoverflow URL: https://stackoverflow.com/questions/34191613/warning-ignoring-return-value-of-fscanf-in-c (дата обращения: 03.11.2022).
 * SIGUSR1 not received // stackoverflow URL: https://stackoverflow.com/questions/14039277/sigusr1-not-received (дата обращения: 03.11.2022).
 */