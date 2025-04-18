# Pedestal Noise Subtraction from ADC Readouts

So, we are trying to subtract pedestal noise from the ADC readouts. There are four methods, and you can find the C++ code for all four methods in this directory. The input ROOT file used is also included here (Merged_File_Run1695831790_NANO.root). But, if the ROOT file is too large to be transferred, you can merge the four ROOT files in the `hgc_root_files` directory using the following command:

```
hadd merged_file_name.root file1.root file2.root file3.root file4.root
```
### Explanation of the Four Methods

#### Method 1:
- **Step 1:** Take the `HGC_adc` branch and subtract `HGC_adcm1` from it.
- **Step 2:** Take the `HGCCM_cm` branch and subtract the channel-wise mean of it.
- **Step 3:** Take the average of two consecutive half-ROCs and subtract that (CM - CMmean) from the `HGC_adc - HGC_adcm1` variable of the respective half-ROCs.

##### Example:
There are 12 half-ROCs (3 HD layers, each having two halves and both sides, so 3 * 2 * 2). Each half-ROC has 37 channels (32 connected, 4 unconnected, and 1 calibration - total 444), with 24 common mode channels.

For Method 1, let's say we take the first half-ROC:
- The `HGC_adc` and `HGC_adcm1` variables will have an array of size 444.
- We then select the first 32 connected channels from the first half-ROC (array indices are given in the codes).
- Subtract the average of the first two common mode channels (channels 0 and 1) from the first half-ROC common mode channels (CM - CMmean) from the HGC_adc-HGC_adcm1.
- Repeat for all the halfrocs.

#### Method 2:
- **Step 1:** Take the `HGC_adc` branch and subtract `HGC_adcm1` from it.
- **Step 2:** Take the `HGCCM_cm` branch and subtract the previous bunch crossing 9previous value) from it.
- **Step 3:** Take the average of two consecutive half-ROCs and subtract that (CM - CMpbc) from the `HGC_adc - HGC_adcm1` variable of the respective half-ROCs.

#### Method 3:
- **Step 1:** Take the `HGC_adc` branch and subtract channelwise mean from it.
- **Step 2:** Calculate the mean of 32 connected channels for each halfroc, subtract it from the `HGC_adc - HGC_adc mean`, the one calculated above

#### Method 4:
- **Step 1:** Take the `HGC_adc` branch and subtract the previous bunch crossing from it.
- **Step 2:** Calculate the mean of 32 connected channels for each halfroc, subtract it from the `HGC_adc - HGC_adc pbc`, the one calculated above
