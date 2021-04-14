# Himax AIoT Platform Board with NB-IoT(TensorFlow Lite for Microcontrollers)
It is a modified version of the [TensorFlow Lite for Microcontrollers](https://github.com/tensorflow/tensorflow/tree/master/tensorflow/lite/micro) for use with Himax-AIoT-NB-G1 Boards. Each example in the package has been tested in Ubuntu 20.04 LTS environment.

Following examples are included :
- person detection INT8 example
  
## Table of contents
  - [Prerequisites](#prerequisites)
  - [Deploy to Himax-AIoT-NB-G1](#deploy-to-h010_hx6539_nb_iot_wnb303r_v10-evb)    
  - [Training your own model](#training-your-own-model)
  - [Convert model from PyTorch to TensorFlow Lite for Microcontrollers](#convert-model-from-pytorch-to-tensorflow-lite-for-microcontrollers)
   
## Prerequisites
- Make Tool version
  
  A `make` tool is required for deploying Tensorflow Lite Micro applications, See
[Check make tool version](https://github.com/tensorflow/tensorflow/blob/master/tensorflow/lite/micro/tools/make/targets/arc/README.md#make-tool)
section for proper environment.

- Development Toolkit
  
  Install the toolkits listed below:

  - GNU Development Toolkit

    See
[ARC GNU Tool Chain](https://github.com/foss-for-synopsys-dwc-arc-processors/toolchain) section for more detail, current released GNU version is [GNU Toolchain for ARC Processors, 2020.09](https://github.com/foss-for-synopsys-dwc-arc-processors/toolchain/releases/download/arc-2020.09-release/arc_gnu_2020.09_prebuilt_elf32_le_linux_install.tar.gz). After download and extract toolkit to local space, please remember to add it to environment PATH. For example:

    ```
    export PATH=[location of your ARC_GNU_ROOT]/bin:$PATH
    ```

- curl command
  
  Installing curl for Ubuntu Linux.
  ```
  sudo apt update
  sudo apt upgrade
  sudo apt install curl
  ```
- Serial Terminal Emulation Application

  There are 2 main purposes for HIMAX WE1 EVB Debug UART port, print application output and burn application to flash by using xmodem send application binary.

## Deploy to Himax-AIoT-NB-G1

The example project for Himax-AIoT-NB-G1 EVB platform can be generated with following command:

Download related third party data and model setting (only need to download once)

```
make download
```

Default building toolchain in makefile is Metaware Development toolkit, if you are trying to build example with GNU toolkit. please change the `ARC_TOOLCHAIN` define in `Makefile` like this

```
ARC_TOOLCHAIN ?= gnu
```

Build person detection INT8 example and flash image, flash image name will be `WEI_FW_gnu_arcem9d_wei_r16.img`

```
make clean
make
make flash 
```

After flash image generated, please download the flash image file to HIMAX WE1 EVB by UART, details are described [here](https://github.com/HimaxWiseEyePlus/Himax-AIoT-NB-G1-SDK-Azure-RTOS-main/Himax-AIoT-NB-G1_user_guide#flash-image-update-at-linux-environment)

## Training your own model

Model used by handwriting example is training with MNIST dataset, please take a look [here](tensorflow/lite/micro/examples/handwriting/training_a_model.md#train-handwriting-model-on-MNIST-dataset) about training flow if you are interested in.


## Convert model from PyTorch to TensorFlow Lite for Microcontrollers

Whenever there is a PyTorch model in your hand, [here](tensorflow/lite/micro/examples/handwriting/pytorch_onnx_tflite/README.md#convert-model-from-pytorch-to-tensorflow-lite-for-microcontrollers) is a tutorial to switch it to tflite model and deploy it on HIMAX WE1 EVB.

