/*
 * task_2.c — c помощью функции sigprocmask() заблокировать получение сигнала SIGUSR1, 
 * после чего восстановить исходную маску сигналов процесса. 
 * Проверить корректность работы получившейся программы. См. стр. 396.
 * 
 * Copyright (c) <2022> <Гордеев Никита>
 * 
 * This code is licensed under a MIT-style license.
 */

/* 
 * #define _XOPEN_SOURCE 700 
 * В соответствии с man 7 feature_test_macros этим макросом (или несколькими другими) может использоваться для 
 * "предотвращения раскрытия нестандартных определений" или "предоставления нестандартных определений, 
 * которые не отображаются по умолчанию".
*/
#define _XOPEN_SOURCE 700
#define _POSIX_SOURCE

#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <signal.h>
#include <stdarg.h> /* vsnprintf() */
#include <fcntl.h>
#include <errno.h>
#include <wchar.h>
#include <locale.h>

// Набор сигналов
sigset_t first_mask, last_mask;

// Функция обработчик сигнала SIGUSR1
void sHandler(int signal);

int main(int argc, char *argv[])
{
    struct sigaction act;
    act.sa_handler = sHandler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;

    // sigaction позволяет проверить действие, связанное с сигналом SIGUSR1
    if (sigaction(SIGUSR1, &act, NULL) == -1) {
        printf("Перехват SIGUSR1 невозможен\n");
        exit(EXIT_FAILURE);
    }

    // raise позволяет процессу послать сигнал себе самому
    if (raise(SIGUSR1) == -1) {
        printf("Ошибка при вызове функции raise\n");
        exit(EXIT_FAILURE);
    }

    // sigemptyset обнуляет целое число
    if (sigemptyset(&last_mask) < 0) {
        printf("Ошибка при вызове функции sigemptyset\n");
        exit(EXIT_FAILURE);
    }

    // Добавление одного сигнала в существующий набор
    if (sigaddset(&last_mask, SIGUSR1) < 0) {
        printf("Ошибка при вызове функции sigaddset\n");
        exit(EXIT_FAILURE);
    }

    // заблокировать SIGUSR1 и сохранить текущую маску сигналов
    if (sigprocmask(SIG_BLOCK, &last_mask, &first_mask) < 0) {
        printf("Ошибка при вызове функции sigprocmask\n");
    } else
        printf("Сигнал был заблокирован\n");
    
    // raise позволяет процессу послать сигнал себе самому
    if (raise(SIGUSR1) == -1) {
        printf("Ошибка при вызове raise\n");
        exit(EXIT_FAILURE);
    }

    // восстановить прежнюю маску сигналов, в которой SIGUSR1 не заблокирован     
    if (sigprocmask(SIG_SETMASK, &first_mask, NULL) < 0) {
        printf("Ошибка при вызове функции sigprocmask\n");
        exit(EXIT_FAILURE);
    } else {
        printf("Сигнал был разблокирован\n");
        exit(EXIT_SUCCESS);
    }

    return 0;
}

// Функция обработчик сигнала SIGUSR1
void sHandler(int signal)
{
    // Форматированный вывод
    if (signal == SIGUSR1)
        printf("Сигнал SIDCHLD принят\n");
    else
        printf("Принят сигнал %d\n", signal);
}

/*
 * Материалы:
 * C - How to correctly use SIGUSR1? // stackoverflow URL: https://stackoverflow.com/questions/50446496/c-how-to-correctly-use-sigusr1 (дата обращения: 03.11.2022).
 * Листинг 10.10. Вывод маски сигналов процесса // «UNIX. Профессиональное программирование. 3-е изд.» Авторы: У. Ричард Стивенс, Стивен А. Раго Год: 2018 URL: https://www.rulit.me/data/programs/resources/pdf/UNIX-Professionalnoe-programmirovanie_RuLit_Me_609965.pdf (дата обращения: 03.11.2022).
 * Глава 10.14. Функция sigaction // «UNIX. Профессиональное программирование. 3-е изд.» Авторы: У. Ричард Стивенс, Стивен А. Раго Год: 2018 URL: https://www.rulit.me/data/programs/resources/pdf/UNIX-Professionalnoe-programmirovanie_RuLit_Me_609965.pdf (дата обращения: 03.11.2022).
 * Глава 10.9. Функции kill и raise // «UNIX. Профессиональное программирование. 3-е изд.» Авторы: У. Ричард Стивенс, Стивен А. Раго Год: 2018 URL: https://www.rulit.me/data/programs/resources/pdf/UNIX-Professionalnoe-programmirovanie_RuLit_Me_609965.pdf (дата обращения: 03.11.2022).
 * Взаимодействие процессов. Сигналы // CyberForum.ru URL: https://www.cyberforum.ru/cpp-linux/thread281344.html (дата обращения: 03.11.2022).
 * Установка обработчика сигналов с использованием сигмации и поднятия сигналов с использованием рейза // POSIX сигналы URL: https://learntutorials.net/ru/posix/topic/4532/сигналы (дата обращения: 03.11.2022).
 * Wait for signal, then continue execution // stackoverflow URL: https://stackoverflow.com/questions/47658347/wait-for-signal-then-continue-execution (дата обращения: 03.11.2022).
 * SIGUSR1 not received // stackoverflow URL: https://stackoverflow.com/questions/14039277/sigusr1-not-received (дата обращения: 03.11.2022).
 * why am i not able to declare sigset_t with std=c99? // stackoverflow URL: https://stackoverflow.com/questions/13618219/why-am-i-not-able-to-declare-sigset-t-with-std-c99 (дата обращения: 03.11.2022).
 * struct sigaction incomplete error // stackoverflow URL: https://stackoverflow.com/questions/6491019/struct-sigaction-incomplete-error (дата обращения: 03.11.2022).
 */