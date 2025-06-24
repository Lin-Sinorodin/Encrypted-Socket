# Kernel Recompile

## Summary

__Consists of 3 steps:__
1. Initial setup (install and update packages, fetch the `linux` source code, initialize kernel config)
2. Modify the kernel source code
3. Recompile and boot with new kernel

__Development process:__
* Do once (setup):
  * The first step performed only once, when initializing a fresh VM
  * Also, it's recommended to compile the kernel once
    * Run only `make -j<N>`
    * After the first compilation, next compilations with `make` will be much faster
* Repeat (development):
  * Repeat steps 2 and 3 - modify the kernel, recompile, and check your changes
  * If the first compilation was done in the setup phase, step 3 should take no longer than a couple of minutes

__Scripts to speed up the process:__
* __Step 1:__
  * Can be done automatically using `/scripts/setup_vm.sh`
* __Step 2:__
  * Usually just copying the code from the development machine to the vm
  * If using shared memory: `sudo /bin/cp -rf /mnt/macos/src /dst/in/vm`
* __Step 3:__
  * The recompilation can be done automatically using `/scripts/rebuild_kernel.sh`
  * Use the script as is only if you are __certain__ that the code compiles!
  * For new (and possibly buggy) code: 
    * After the first compilation, next compilations with `make` will be much faster
    * Therefore, it's better to run `make -j<N>` before the script
    * See that your modification compiles (usually the modified files runs first)
    * Then just CTRL+C and run the scrit to run the full process automatically

---

## Step 1 - Setup

### Install Prerequisites

```bash
sudo apt update
sudo apt install build-essential libncurses5-dev bison flex libssl-dev libelf-dev libudev-dev libpci-dev libiberty-dev fakeroot git
```

### Get the Kernel Source

* Use Debian’s Kernel Source:

    ```bash
    sudo apt install linux-source
    cd /usr/src
    tar -xvf linux-source-*.tar.xz
    rm -f linux-source-*.tar.xz
    cd linux-source-*
    ```

### Setup Kernel Config

- Copy current config:

    ```bash
    cp /boot/config-$(uname -r) .config
    ```

- Then, update the config for current source:

    ```bash
    make olddefconfig
    ```

- Disable UEFI certificate signing in `.config`
    - (Search and set these options manually):
      `CONFIG_MODULE_SIG=n`
    - Or use `sed`:

        ```bash
        sed -i s/CONFIG_MODULE_SIG=y/CONFIG_MODULE_SIG=n/g .config
        ```

- Disable BTF in `.config`
    - (Search and set these options manually):
      `CONFIG_DEBUG_INFO_BTF=n`
    - Or use `sed`:

        ```bash
        sed -i s/CONFIG_DEBUG_INFO_BTF=y/CONFIG_DEBUG_INFO_BTF=n/g .config
        ```

## Step 2 - Modify the Kernel

* At this point, edit the source code as needed. 
* Check with a POC example:
  * Go to **`init/main.c`**
  * Inside that function, near the top (but after `pr_notice()`), add this line:
    ```c
    printk(KERN_INFO "Custom kernel booted successfully!\n");`
    ```

## Step 3 - Recompile and Boot

### Compile the Kernel

* This will take some time (in the first time). 
* Can speed it up with `-j$(nproc)`:
    ```bash
    make -j$(nproc)
    make modules -j$(nproc)
    ```

### Install Modules and Kernel

* Install modules (they’ll go to `/lib/modules/<your-version>`):
    ```bash
    sudo make modules_install
    ```

* Install the kernel:
    ```bash
    sudo make install
    ```

* This:
  - Installs `vmlinuz-*` to `/boot`
  - Installs `System.map-*`
  - Updates GRUB to include your new kernel

### [Optional] Update GRUB and `Initramfs`

* **At the moment, no need for this step**
* This is often done automatically after `make install`
* Look up into this commands if something went wrong:
    ```bash
    sudo update-initramfs -c -k <new-kernel-version>
    sudo update-grub
    ```

### Reboot and Select Your New Kernel

* Reboot the machine:
    ```bash
    sudo reboot
    ```

* Usually, the new kernel will be used automatically on the next time the machine boots up
* If not, when the GRUB menu appears, choose _Advanced options_, and select the new kernel

### Check New Kernel

* Check the kernel version:
    ```bash
    uname -r
    ```

* Check the POC example from step 2:
  * Run `dmesg` to search for the print we added to the kernel init function:

      ```bash
      dmesg | grep "Custom"
      ```

  * And the message from the custom kernel should be displayed:

    ![image.png](/assets/setup_poc.png)