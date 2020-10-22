#include "unistd.h"
#include "stdlib.h"
#include "sys/wait.h"
#include "string.h"

#define CD_BAD_ARG "error: cd: bad arguments\n"
#define CD_CH_DIR "error: cd: cannot change directory to "
#define FATAL "error: fatal\n"
#define EXEC "error: cannot execute "
#define PIPE 1
#define NONE 0

typedef struct      s_data
{
    char            **arg;
    int             type;
    int             pipe[2];
    struct s_data   *next;
    struct s_data   *prev;
}                   t_data;

int ft_strlen(char *str)
{
    int count = 0;
    
    while (*str++)
        count++;
    return count;
}

void ft_str_error(char *str)
{
    write(2, str, ft_strlen(str));
}

void ft_fatal_error()
{
    ft_str_error(FATAL);
    exit(1);
}

void ft_str_error_arg(char *str, char *arg)
{
    ft_str_error(str);
    ft_str_error(arg);
    ft_str_error("\n");
}

char *ft_strdup(char *str)
{
    char *new;
    int i = 0;
    
    if (!(new = (char *)malloc(ft_strlen(str) + 1)))
        ft_fatal_error();
    while (str[i])
    {
        new[i] = str[i];
        i++;
    }
    new[i] = '\0';
    return new;
}

t_data *ft_lstnew(int type, int count, char **arg)
{
    t_data *new;
    int i = 0;
    
    if (!(new = (t_data *)malloc(sizeof(t_data))))
        ft_fatal_error();
    new->next = NULL;
    new->prev = NULL;
    new->type = type;
    if (!(new->arg = (char **)malloc((sizeof(char *) * (count + 1)))))
        ft_fatal_error();
    while (arg[i] && count--)
    {
        new->arg[i] = ft_strdup(arg[i]);
        i++;
    }
    new->arg[i] = NULL;
    return new;
}

void    ft_pushback(t_data **head, int type, int count, char **arg)
{
    t_data *tmp;
    
    tmp = *head;
    if (!*head)
        *head = ft_lstnew(type, count, arg);
    else
    {
        while (tmp->next)
            tmp = tmp->next;
        tmp->next = ft_lstnew(type, count, arg);
        tmp->next->prev = tmp;
    }
}

void    ft_call_cd(char **arg)
{
    int i = 0;
    while (arg[i])
    {
        i++;
    }
    if (i != 2)
        ft_str_error(CD_BAD_ARG);
    else
    {
        if (chdir(arg[1]) < 0)
            ft_str_error_arg(CD_CH_DIR, arg[1]);
    }
}

void    ft_free_lst(t_data **head)
{
    t_data *tmp = *head;
    t_data *tmp_ptr;
    int i;
    
    while (tmp)
    {
        i = 0;
        while (tmp->arg[i])
        {
            free(tmp->arg[i]);
            i++;
        }
        free(tmp->arg);
        tmp_ptr = tmp->next;
        free(tmp);
        tmp = tmp_ptr;
    }
}

void ft_call_exec_func(t_data *cmd, char **arg)
{
    pid_t pid;
    int status;
    
    if (cmd->type == PIPE && pipe(cmd->pipe) < 0)
        ft_str_error(FATAL);
    pid = fork();
    if (pid < 0)
        ft_str_error(FATAL);
    else if (pid == 0)
    {
        if (cmd->type == PIPE && dup2(cmd->pipe[1], 1) < 0)
            ft_str_error(FATAL);
        if (cmd->prev && cmd->prev->type == PIPE && dup2(cmd->prev->pipe[0], 0) < 0)
            ft_str_error(FATAL);
        if (execve(cmd->arg[0], cmd->arg, arg) < 0)
            ft_str_error_arg(EXEC, cmd->arg[0]);
        exit(0);
    }
    else
        waitpid(pid, &status, 0);
    if (cmd->type == PIPE)
        close(cmd->pipe[1]);
    if (cmd->prev && cmd->prev->type == PIPE)
        close(cmd->prev->pipe[0]);
}

void    ft_call_exec(t_data *head, char **arg)
{
    while (head)
    {
        if (!strcmp(head->arg[0], "cd"))
            ft_call_cd(head->arg);
        else
            ft_call_exec_func(head, arg);
        head = head->next;
    }
}

int main(int ac, char **av, char **env)
{
    t_data *head = NULL;
    int start = 1;
    int count = 0;
    int type;
    
    while (++count < ac)
    {
        if (!strcmp(av[count], "|") || !strcmp(av[count], ";") || count + 1 == ac)
        {
            if (!strcmp(av[count], "|"))
                type = PIPE;
            else if (!strcmp(av[count], ";"))
                type = NONE;
            else
            {
                type = NONE;
                count++;
            }
            if (count-start != 0)
                ft_pushback(&head, type, count-start, &av[start]);
            start = count + 1;
        }
    }
    ft_call_exec(head, env);
    ft_free_lst(&head);
}
