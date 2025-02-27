To save spec as PDF - select text below (including images) -> right click -> Print -> Save as PDF

# Software
#### Spec revision № 1.0
This spec outlines the software part of the Open Source Time Appliance,
including driver, NTP and PTP servers

## Table of Contents
1. [Driver](#Driver)
1. [NTP](#NTP)
   1. [Chrony](#chrony)
1. [PTP](#PTP)
   1. [Server](#server)
      1. [ptp4u](#ptp4u)
      1. [linuxptp](#linuxptp)
   1. [Monitoring](#monitoring)
      1. [ptpcheck](#ptpcheck)
1. [License](#License)


## Driver
This repository contains the [ocp_ptp driver](https://github.com/opencomputeproject/Time-Appliance-Project/tree/master/Time-Card/DRV) (included in Linux kernel 5.12 and newer). Driver may require vt-d CPU flag enabled in BIOS.
To compile the driver manually just run `./remake` and then load it with `modprobe ocp_ptp`.  
It will expose all the devices like this:

```
$ ls -g /sys/class/timecard/ocp0/
total 0
-r--r--r--. 1 root 4096 Aug  3 19:49 available_clock_sources
-rw-r--r--. 1 root 4096 Aug  3 19:49 clock_source
lrwxrwxrwx. 1 root    0 Aug  3 19:49 device -> ../../../0000:04:00.0/
-r--r--r--. 1 root 4096 Aug  3 19:49 gnss_sync
lrwxrwxrwx. 1 root    0 Aug  3 19:49 i2c -> ../../xiic-i2c.1024/i2c-2/
drwxr-xr-x. 2 root    0 Aug  3 19:49 power/
lrwxrwxrwx. 1 root    0 Aug  3 19:49 pps -> ../../../../../virtual/pps/pps1/
lrwxrwxrwx. 1 root    0 Aug  3 19:49 ptp -> ../../ptp/ptp2/
-r--r--r--. 1 root 4096 Aug  3 19:49 serialnum
lrwxrwxrwx. 1 root    0 Aug  3 19:49 subsystem -> ../../../../../../class/timecard/
lrwxrwxrwx. 1 root    0 Aug  3 19:49 ttyGNSS -> ../../tty/ttyS7/
lrwxrwxrwx. 1 root    0 Aug  3 19:49 ttyMAC -> ../../tty/ttyS8/
-rw-r--r--. 1 root 4096 Aug  3 19:39 uevent
```

OCP Time Appliance can be used with any NTP/PTP server.  
Here are couple of examples and configs:
## NTP
NTP servers usually can work directly with PHC
### Chrony
https://github.com/mlichvar/chrony
```
$ cat /etc/chrony.conf
refclock PHC /dev/ptp2 tai poll 0 trust
...
leapsectz right/UTC
allow all
hwtimestamp *
```

## PTP
Keep in mind synchronization of PHC between time card and the Network card is required.
The example shown is for unicast server only.
### Server
#### ptp4u
https://github.com/facebook/time/ptp
```
$ /usr/local/bin/ptp4u -iface eth1
```

#### linuxptp
https://sourceforge.net/projects/linuxptp/
```
$ /sbin/ptp4l -f /etc/ptp4l.conf -i eth0 -m
```
### Monitoring
#### ptpcheck
https://github.com/facebook/time/ptp
ptpcheck is the tool allowing to debug ptp server and a client.  
Server:
```
$ /usr/local/bin/ptpcheck trace -S <server>
INFO[0000] using ClockIdentity 1234dd.fffe.5678aa, talking to <server> using Two-Step Unicast PTPv2 protocol
...
Collected measurements:
      N|       0 |       1 |       2 |       3 |
  delay| 7.907µs | 7.903µs | 7.923µs | 7.907µs |
 offset| 9.558µs | 9.648µs | 9.689µs | 9.941µs |
INFO[0015] done
```
Client:
```
$ /usr/local/bin/ptpcheck diag
[ OK ] GM is present
[ OK ] Period since last ingress is 600.804116ms, we expect it to be within 1s
[ OK ] GM offset is -71ns, we expect it to be within 250µs
[ OK ] GM mean path delay is 3.622µs, we expect it to be within 100ms
```

# License
Contributions to this Specification are made under the terms and conditions set forth in Open Web Foundation Contributor License Agreement (“OWF CLA 1.0”) (“Contribution License”) by: 
 
 Facebook

You can review the signed copies of the applicable Contributor License(s) for this Specification on the OCP website at http://www.opencompute.org/products/specsanddesign 
Usage of this Specification is governed by the terms and conditions set forth in Open Web Foundation Final Specification Agreement (“OWFa 1.0”) (“Specification License”).   
 
You can review the applicable Specification License(s) executed by the above referenced contributors to this Specification on the OCP website at http://www.opencompute.org/participate/legal-documents/
 Notes: 
 
1)     The following clarifications, which distinguish technology licensed in the Contribution License and/or Specification License from those technologies merely referenced (but not licensed), were accepted by the Incubation Committee of the OCP:  
 
None

 
NOTWITHSTANDING THE FOREGOING LICENSES, THIS SPECIFICATION IS PROVIDED BY OCP "AS IS" AND OCP EXPRESSLY DISCLAIMS ANY WARRANTIES (EXPRESS, IMPLIED, OR OTHERWISE), INCLUDING IMPLIED WARRANTIES OF MERCHANTABILITY, NON-INFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, OR TITLE, RELATED TO THE SPECIFICATION. NOTICE IS HEREBY GIVEN, THAT OTHER RIGHTS NOT GRANTED AS SET FORTH ABOVE, INCLUDING WITHOUT LIMITATION, RIGHTS OF THIRD PARTIES WHO DID NOT EXECUTE THE ABOVE LICENSES, MAY BE IMPLICATED BY THE IMPLEMENTATION OF OR COMPLIANCE WITH THIS SPECIFICATION. OCP IS NOT RESPONSIBLE FOR IDENTIFYING RIGHTS FOR WHICH A LICENSE MAY BE REQUIRED IN ORDER TO IMPLEMENT THIS SPECIFICATION.  THE ENTIRE RISK AS TO IMPLEMENTING OR OTHERWISE USING THE SPECIFICATION IS ASSUMED BY YOU. IN NO EVENT WILL OCP BE LIABLE TO YOU FOR ANY MONETARY DAMAGES WITH RESPECT TO ANY CLAIMS RELATED TO, OR ARISING OUT OF YOUR USE OF THIS SPECIFICATION, INCLUDING BUT NOT LIMITED TO ANY LIABILITY FOR LOST PROFITS OR ANY CONSEQUENTIAL, INCIDENTAL, INDIRECT, SPECIAL OR PUNITIVE DAMAGES OF ANY CHARACTER FROM ANY CAUSES OF ACTION OF ANY KIND WITH RESPECT TO THIS SPECIFICATION, WHETHER BASED ON BREACH OF CONTRACT, TORT (INCLUDING NEGLIGENCE), OR OTHERWISE, AND EVEN IF OCP HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
