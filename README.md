# Encrypted Socket

## Summary

Encrypted socket implemented in the Linux kernel.

## Environment

### Tested on

* Mac mini with an M1 Apple silicone 
* Linux VM running on UTM:
  * Distribution: `Debian 12.2.0-14` from [UTM gallery](https://mac.getutm.app/gallery/debian-12)
  * Kernel version (`uname -r`): `6.1.0-13-arm64`

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