/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   microshell.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: akretov <akretov@student.42prague.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/03 15:50:39 by akretov           #+#    #+#             */
/*   Updated: 2024/11/03 18:47:52 by akretov          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>

void ft_putstr_fd2(const char *str, const char *arg)
{
	while (*str)
		write(2, str++, 1);
	if (arg)
		while (*arg)
			write(2, arg++, 1);
	write(2, "\n", 1);
}

void ft_execute(char *argv[], int tmp_fd, char *env[])
{
	if (execve(argv[0], argv, env) == -1)
	{
		ft_putstr_fd2("error: cannot execute ", argv[0]);
		exit(1);
	}
}

void handle_cd(char *argv[])
{
	if (argv[1] == NULL || argv[2] != NULL)
		ft_putstr_fd2("error: cd: bad arguments", NULL);
	else if (chdir(argv[1]) != 0)
		ft_putstr_fd2("error: cd: cannot change directory to ", argv[1]);
}

void handle_pipe(char *argv[], int *tmp_fd, char *env[])
{
	int fd[2];
	if (pipe(fd) == -1)
	{
		ft_putstr_fd2("error: pipe creation failed", NULL);
		return;
	}
		
	if (fork() == 0)
	{
		dup2(fd[1], STDOUT_FILENO);
		close(fd[0]);
		close(fd[1]);
		ft_execute(argv, *tmp_fd, env);
	}
	else
	{
		close(fd[1]);
		close(*tmp_fd);
		*tmp_fd = fd[0];
	}
}

void handle_semicolon(char *argv[], int *tmp_fd, char *env[])
{
	if (fork() == 0)
		ft_execute(argv, *tmp_fd, env);
	else
	{
		close(*tmp_fd);
		while (waitpid(-1, NULL, 0) != -1);
		*tmp_fd = dup(STDIN_FILENO);
	}
}

void process_commands(char *argv[], char *env[])
{
	int	i = 0;
	int	tmp_fd = dup(STDIN_FILENO);
	int	start = 0;

	while (argv[i])
	{
		start = i;
		while (argv[i] && strcmp(argv[i], ";") && strcmp(argv[i], "|"))
			i++;
		if (strcmp(argv[start], "cd") == 0)
			handle_cd(&argv[start]);
		else
		{
			if (i > start)
			{
				if (argv[i] == NULL || strcmp(argv[i], ";") == 0)
					handle_semicolon(&argv[start], &tmp_fd, env);
				else if (strcmp(argv[i], "|") == 0)
					handle_pipe(&argv[start], &tmp_fd, env);
			}
		}
		if (argv[i])
			i++;
	}
	close(tmp_fd);
}

int main(int argc, char *argv[], char *env[])
{
	if (argc > 1)
		process_commands(argv + 1, env);
	return (0);
}
