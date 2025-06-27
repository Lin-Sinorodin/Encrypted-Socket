# Encrypted Socket

## Summary

Encrypted socket implemented in the Linux kernel:
* The encryption process is done internally by the (modified) linux kernel
* The user can just use the normal socket syscalls (with a slight modification detailed below)
* This repo provides detailed setup instructions, POC scripts, and implementation details

## Environment

### Tested on

* Mac mini with an M1 Apple silicone 
* Linux VM running on UTM:
  * Distribution: `Debian 12.2.0-14` from [UTM gallery](https://mac.getutm.app/gallery/debian-12)
  * Kernel version (`uname -r`): `6.1.0-13-arm64`
  * Corresponds to [linux v6.1.140](https://git.kernel.org/pub/scm/linux/kernel/git/stable/linux.git/tree/net?h=v6.1.140)

## Setup

* This projects involves adding new features to the linux kernel
* As this is a complicated process, it's described in a dedicated page: [Recompile Kernel](recompile.md)
* This page starts with `summary` section - a brief description of:
  * The recompilation steps
  * The development process
  * Useful scripts
* __Don't skip the `summary` section__
  * it includes a lot of useful tips, that will save a lot of time (and pain)
* The page includes a POC example:
  * Allows to check that the compilation runs properly with a small modification
  * If possible, run the POC first and check it's working before doing more heavy modifications

## Intended Usage

### Concept

* Assumes 2 machines, both with modified kernel
* The main idea behind this implementation, is to keep it hidden from the user
* The only thing changed here, is the addition of a flag to the socket initialization
  * __Client:__ Initializes a socket, with the secret `ENCRYPT` flag
      ```c
      int fd = socket(AF_INET, SOCK_STREAM | (1 << SOCK_ENCRYPT), 0);
      ```
  * __Server:__ Initializes a socket, with the secret `DECRYPT` flag
      ```c
      int fd = socket(AF_INET, SOCK_STREAM | (1 << SOCK_DECRYPT), 0);
      ```
### POC Scripts

* To demonstrate the simplicity of this method, minimal scripts for client and server are provided
* One can notice that this scripts looks like any other basic server/client script in `c`!
* As mentioned in the concept, the only thing special here is flags in initialization
* Both script defines the `IP` and `PORT` hardcoded as `define` at the top, __modify before compiling!__
* __Client:__
  * Uses the `ENCRYPT` socket, sending a message encrypted
  * Path: [`/socket_scripts/client.c`](/socket_scripts/client.c)
  * Compile: `gcc /socket_scripts/client.c -o client`
  * Run: `sh ./client`
* __Server:__
  * Uses the `DECRYPT` socket, receiving a message decrypted
  * Path: [`/socket_scripts/server.c`](/socket_scripts/server.c)
  * Compile: `gcc /socket_scripts/server.c -o server`
  * Run: `sh ./server`

### Example (POC Scripts)

* Initialized a vm with the modified kernel, following the instructions above
* Cloned the vm, and changed the mac address of the cloned machine (to get a new `IP` address)
* defined second machine as server (receiving) and found its IP with:
  ```bash
  ip -4 addr show scope global | grep inet | awk '{print $2}'
  ```
* Followed this steps (in this order):
  * Copied the client/server scripts for each machine and compiled it (after updating `IP` in both scripts)
  * __Server:__ 
    * Started the server `sh ./server`
    * The server in listening mode, waiting for connection
  * __Client:__
    * Launched `tcpdump` to sniff the message:
      ```bash
      sudo tcpdump -A -X tcp and dst 192.168.64.14 and "tcp[tcpflags] & tcp-ack == 0"
      ```
    * Started the client `sh ./client`
    * Checked the kernel logs with `sudo dmesg`
* __Result:__
  * __Client:__
    * The client script sent `Hello from client`:
      ![send](/assets/example_send.png)
    * The message was encrypted by the kernel, with key `35` (`dmesg`):
      ![send_kernel](/assets/example_send_kernel.png)
    * The message cought by `tcpdump` is clearly encryped:
      ![send_tcpdump](/assets/example_tcpdump.png)
  * __Server:__
    * The server reconstructed the message:
      ![recv](/assets/example_recv.png)
    * The message was decrypted by the kernel (`dmesg`):
      ![recv_kernel](assets/example_recv_kernel.png)

## Implementation

* All the modifications was added over the __`net/socket.c`__ file in the kernel code
* I added the original file in an initial commit, so it's easy to see the modifications with git diff
* The added code is quiet documented, but a more comprehensive documentation will be added here
* __The modified code located here:__ [`/kernel/socket.c`](/kernel/socket.c)
* A scheme draw of the implementation is at [`/assets/implementation.drawio`](/assets/implementation.drawio) 
and can be edited online with [drawio](https://www.drawio.com/)

### Technical details

* The code implements a way to communicate between 2 sockets in an encrypted way
* The process assumed connection based sockets (`TCP`)
* The process is as detailed in this figure:
  ![/assets/implementation.svg](/assets/implementation.svg)
* Most of the socket module kept untouched, only the modifications will be documented below
* __Client:__
  * `socket()`
    * [`User`] The user creates a `TCP` socket with encrypt flag, passed to the `type` field
      ```c
      int fd = socket(AF_INET, SOCK_STREAM | (1 << SOCK_ENCRYPT), 0);
      ```
    * [`Kernel`] Generates random one char encryption key
    * [`Kernel`] store the encryption flag and key in the flags field of `struct socket`
  * `connect()`
    * [`Kernel`] Creates a raw socket
    * [`Kernel`] Send a ping (`ICMP Echo`) with key as its payload to the server
  * `send()`
    * [`User`] Send a message:
      ```c
      char msg[] = "Hello from client";
      send(fd, msg, sizeof(msg), 0);
      ```
    * [`Kernel`] Encrypts the data using byte-wise xor with the key
* __Server:__
  * `socket()`
    * [`User`] The user creates a `TCP` socket with decrypt flag, passed to the `type` field
      ```c
      int fd = socket(AF_INET, SOCK_STREAM | (1 << SOCK_DECRYPT), 0);
      ```
    * [`Kernel`] store the decryption flag in the flags field of `struct socket`
  * `accept()`
    * [`Kernel`] Creates a raw socket
    * [`Kernel`] Recv a ping (`ICMP Echo`) with key as its payload from the client
    * [`Kernel`] store the encryption key in the flags field of `struct socket`
  * `recv()`
    * [`User`] Receive a message:
      ```c
      char buff[BUFF];
      bzero(buff, sizeof(buff));
      recv(fd, buff, sizeof(buff), 0);
      ```
    * [`Kernel`] Decrypts the data using byte-wise xor with the key
* This way:
  * The user uses the regular socket interface with almost no changes
  * The data is encrypted secretly and effortlessly by the kernel socket handling

## Useful tools

* Linux stable source code [linux v6.1.140](https://git.kernel.org/pub/scm/linux/kernel/git/stable/linux.git/tree/net?h=v6.1.140)
* [bootlin](https://elixir.bootlin.com/linux/v6.1.140/source/net/socket.c) -
  Elixir Cross Referencer, great for navigating the code
* How to make a [private fork in GitHub](https://stackoverflow.com/questions/10065526/github-how-to-make-a-fork-of-public-repository-private/63680732#63680732)
* man pages for the socket syscalls (
  [`socket(2)`](https://man7.org/linux/man-pages/man2/socket.2.html)
  ,[`connect(2)`](https://man7.org/linux/man-pages/man2/connect.2.html), etc.)
* [A Guide to Using Raw Sockets](https://www.opensourceforu.com/2015/03/a-guide-to-using-raw-sockets/)

## Next steps

- [ ] Allow both sides (server and client) to encrypt and decrypt
- [ ] Add a reply in the `connect`-`accept` to notify the client the server actually got the key
- [ ] Add support for better encryption algorithms instead of byte-wise xor