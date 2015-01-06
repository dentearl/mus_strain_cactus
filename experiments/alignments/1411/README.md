# Alignment for Release 1411
This run used the version of progressiveCactus saved under the tag [msca_1411](https://github.com/glennhickey/progressiveCactus/releases/tag/msca_1411).

Sequences can be found [here](ftp://ftp-mouse.sanger.ac.uk/REL-1411-Assembly/).

Command-line arguments used:

```
 --config cactus_progressive_config.xml --batchSystem parasol --bigBatchSystem singleMachine --defaultMemory 8589934593 --bigMemoryThreshold 8589934592 --bigMaxMemory 893353197568 --bigMaxCpus 25 --maxThreads 25 --parasolCommand='/cluster/home/jcarmstr/bin/parasol -host=ku' --retryCount 3 --rootOutgroupPaths criGri1.fa,jacJac1.fa,hetGla2.fa --rootOutgroupDists 0.2689,0.45,0.508 1411.txt work 1411.hal
```
