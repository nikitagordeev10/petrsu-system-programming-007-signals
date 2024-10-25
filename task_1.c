/*
 * task_1.c — С использованием функции sigaction() назначить обработчик сигнала SIGCHLD. 
 * В обработчике вызывать для завершившихся процессов функцию из семейства wait(). 
 * Проверить корректность работы получившейся программы.
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
// #include <linux/signal.h>

pid_t pid;
int status;

// Функция обработчик сигнала SIGCHLD
void sHandler(int signal);

int main(int argc, char *argv[])
{
    // Проверка на наличие аргументов
    if (argc != 2) {
        fprintf(stderr, "Использование: ./task_1 test\n");
        exit(EXIT_FAILURE);
    }

    struct sigaction act;
    act.sa_handler = sHandler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;

    // Вызов функции sigaction
    if (sigaction(SIGCHLD, &act, NULL) == -1) {
        printf("Ошибка функции sigaction\n");
        exit(EXIT_FAILURE);
    }

    // Создаём дочерний процесс
    if ((pid = fork()) < 0) {
        printf("Ошибка вызова функции fork\n");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        printf("Запущен процесс потомок\n");
        if (execl(argv[1], argv[1], NULL) < 0) {
            printf("Ошибка функции запуска файла на исполнение execl\n");
            exit(EXIT_FAILURE);
        }
    }

    // Дождаться доставки сигнала 
    pause();

    return 0;
}

// Функция обработчик сигнала SIGCHLD
void sHandler(int signal)
{
    // Проверка получения сигнала
    if (signal == SIGCHLD) {
        printf("Cигнал sigchld был принят\n");
    } else
        printf("Сигнал %d принят\n", signal);
    
    // Вызываем для завершившихся процессов функцию из семейства wait()
    // Проверка кода заверешения дочернего процесса    
    if (waitpid(pid, &status, WNOHANG) != pid) {
        printf("Ошибка функции waitpid\n");
        exit(EXIT_FAILURE);
    } else if (WIFEXITED(status) != 0) {
        printf("Успешное завершение процесса\n");
        printf("Код выхода: %d\n", WEXITSTATUS(status));
        exit(EXIT_SUCCESS);
    } else if (WIFSIGNALED(status) != 0) {
        printf ("Аварийное завершение процесса\n");
        printf("Код выхода: %d\n", WTERMSIG(status));
        exit(EXIT_SUCCESS);
    }
}


/*
 * Материалы:
 * Глава 10.14. Функция sigaction // «UNIX. Профессиональное программирование. 3-е изд.» Авторы: У. Ричард Стивенс, Стивен А. Раго Год: 2018 URL: https://www.rulit.me/data/programs/resources/pdf/UNIX-Professionalnoe-programmirovanie_RuLit_Me_609965.pdf (дата обращения: 03.11.2022).
 * Листинг 10.12. Реализация функции signal на основе функции sigaction // «UNIX. Профессиональное программирование. 3-е изд.» Авторы: У. Ричард Стивенс, Стивен А. Раго Год: 2018 URL: https://www.rulit.me/data/programs/resources/pdf/UNIX-Professionalnoe-programmirovanie_RuLit_Me_609965.pdf (дата обращения: 03.11.2022).
 * sigaction(2) // manpages.org URL: https://ru.manpages.org/sigaction/2 (дата обращения: 03.11.2022).
 * Практика работы с сигналами // Хабр URL: https://habr.com/ru/post/141206/ (дата обращения: 03.11.2022).
 * struct sigaction incomplete error // stackoverflow URL: https://stackoverflow.com/questions/6491019/struct-sigaction-incomplete-error (дата обращения: 03.11.2022).
 * Глава 10.11. Наборы сигналов // «UNIX. Профессиональное программирование. 3-е изд.» Авторы: У. Ричард Стивенс, Стивен А. Раго Год: 2018 URL: https://www.rulit.me/data/programs/resources/pdf/UNIX-Professionalnoe-programmirovanie_RuLit_Me_609965.pdf (дата обращения: 03.11.2022).
 * How can I handle SIGCHLD? // stackoverflow URL: https://stackoverflow.com/questions/7171722/how-can-i-handle-sigchld (дата обращения: 03.11.2022).
 * Недопустимый неполный тип // CyberForum.ru URL: https://www.cyberforum.ru/cpp-beginners/thread2458171.html (дата обращения: 03.11.2022).
 * why am i not able to declare sigset_t with std=c99? // stackoverflow URL: https://stackoverflow.com/questions/13618219/why-am-i-not-able-to-declare-sigset-t-with-std-c99 (дата обращения: 03.11.2022).
 * struct sigaction incomplete error // stackoverflow URL: https://stackoverflow.com/questions/6491019/struct-sigaction-incomplete-error (дата обращения: 03.11.2022).
 */