# BiomedBench
## General Information
BiomedBench is an open-source benchmark suite of TinyML biomedical applications targeting low-power wearbles [1].

This repository contains a set of biomedical applications designed to run in low-power wearable platforms for patient monitoring.

All applications are coded in C/C++. For each application, we include a Desktop version and the ported versions to different commercial platforms. 

## Applications
- ### Heartbeat Classifier (HeartBeatClass)
    The HeartBeatClass [2] detects abnormal beating patterns in real time for common heart diseases using the ECG signal. For further details check [1].
- ### Seizure Detector SVM (SeizureDetSVM)
    The SeizureDetSVM [3] works on ECG input and recognizes real-time epileptic episodes. For further details check [1].
- ### Seizure Detector CNN (SeizureDetCNN)
    The SeizureDetCNN [4] is based on EEG data and detects real-time epileptic seizure episodes. For further details check [1].
- ### Cognitive Workload Monitor (CognWorkMon)
    The CognWorkMon [5] is designed for real-time monitoring of the cognitive workload state of a subject and is based on EEG input. For further details check [1].
- ### Gesture Classifier (GestureClass)
    The GestureClass [6] aims to classify hand gestures by inspecting signals captured by sEMG of the forearm. For further details check [1].
- ### Cough Detector (CoughDet)
    The CoughDet [7] is a novel application using non-invasive chest-worn biosensors to count the number of cough episodes people experience per day, thus providing a quantifiable means of evaluating the efficacy of chronic cough treatment. For further details check [1].
- ### Emotion Classifier (EmotionClass)
    The EmotionClass [8] classifies patients’ fear status to prevent gender-based violence based on three physiological signals: Galvanic skin response (GSR), PPG, and skin temperature (ST). For further details check [1].
- ### Biological backpropagation-free (BioBPfree)
    BioBPfree [9] is the only benchmark that performs on-device training. For further details check [1].


## Considered Platforms
Currently, the considered boards and their MCUs are:
- [Raspberry Pi Pico](https://datasheets.raspberrypi.com/pico/pico-datasheet.pdf) featuring [RP2040](https://datasheets.raspberrypi.com/rp2040/rp2040-datasheet.pdf)
- [Nucleo-L4R5ZI](https://www.st.com/resource/en/user_manual/um2179-stm32-nucleo144-boards-mb1312-stmicroelectronics.pdf) featuring [STM32L4R5ZI](https://www.st.com/resource/en/datasheet/stm32l4r5vi.pdf)
- [Ambiq Apollo3 Blue AMA3BEVB](https://ambiq.com/wp-content/uploads/2020/10/Apollo3-Blue-SoC-Product-Brief.pdf) featuring the [Apollo 3 Blue](https://ambiq.com/wp-content/uploads/2020/10/Apollo3-Blue-SoC-Datasheet.pdf)
- [Gapuino](https://gwt-website-files.s3.eu-central-1.amazonaws.com/gapuino_v1.1_um.pdf) featuring [GAP8](https://gwt-website-files.s3.amazonaws.com/gap8_datasheet.pdf)
- [GAP9_EVK](https://greenwaves-technologies.com/wp-content/uploads/2024/05/GAP9_EVK-User-Manual_Rel2.4-1.pdf) featuring [GAP9](https://greenwaves-technologies.com/wp-content/uploads/2024/05/gap9datasheet_v1.0.pdf)


## Repository Structure
The folder structure look like that:
```
├── Applications/
|   ├── <App_name>/
|   |   ├── <App_version>/

|   |   |   ├── single_core/
|   |   |   |   ├── <platform_name>
|   |   |   |   |   ├── Inc/
|   |   |   |   |   ├── Src/
|   |   |   |   |   ├── Makefile
|   |   |   |   |   ├── Readme.md

|   |   |   ├── multicore/
|   |   |   |   ├── <platform_name>
|   |   |   |   |   ├── Inc/
|   |   |   |   |   ├── Src/
|   |   |   |   |   ├── Makefile
|   |   |   |   |   ├── Readme.md
```

Look at the Readme of each platform folder (./Applications/.../<platform_name>/Readme.md) for more information on how to run the application on each platform.

## Issues and Troubleshooting
If you find any problems or issues with the applications, please check out the [issue tracker](https://github.com/esl-epfl/biomedbench/issues) and create a new issue if your problem is not yet tracked.

## Useful links
For an overview of BiomedBench, check out: [BiomedBench website](https://biomedbench.epfl.ch/)

For more details, check out the paper preprint: [BiomedBench article](https://infoscience.epfl.ch/record/311495?v=pdf)

## References

1. [Samakovlis, Dimitrios, et al. "BiomedBench: A benchmark suite of TinyML biomedical applications for low-power wearables", 2024.](https://infoscience.epfl.ch/record/311495?v=pdf)
2. [Rubén Braojos, Giovanni Ansaloni, and David Atienza. 2013. A methodology for embedded classification of heartbeats using random projections. In 2013 Design, Automation & Test in Europe Conference & Exhibition (DATE). IEEE, Grenoble, France, 899-904.](https://doi.org/10.7873/DATE.2013.189)
3. [Farnaz Forooghifar, Amir Aminifar, and David Atienza Alonso. 2018. Self-Aware Wearable Systems in Epileptic Seizure Detection. In DSD 2018. IEEE, Prague, Czech Republic, 426-432.](https://doi.org/10.1109/DSD.2018.00078)
4. [Catalina Gomez, Pablo Arbelaez, Miguel Navarrete, Catalina Alvarado-Rojas, Michel Le Van Quyen, and Mario Valderrama. 2020. Automatic seizure detection based on imaged-EEG signals through fully convolutional networks. Scientific Reports 10 (12 2020).](https://doi.org/10.1038/s41598-020-78784-3)
5. [Renato Zanetti, Adriana Arza, Amir Aminifar, and David Atienza. 2022. Real-Time EEG-Based Cognitive Workload Monitoring on Wearable Devices.IEEE Trans. Biomed. Eng. 69, 1 (2022), 265-277.](https://doi.org/10.1109/TBME.2021.3092206)
6. [Mattia Orlandi, Marcello Zanghieri, Victor Javier Kartsch Morinigo, Francesco Conti, Davide Schiavone, Luca Benini, and Simone Benatti. 2022. sEMG Neural Spikes Reconstruction for Gesture Recognition on a Low-Power Multicore Processor. In 2022, IEEE Biomedical Circuits and SystemsConference (BioCAS). IEEE, Taipei, Taiwan, 704-708. ](https://doi.org/10.1109/BioCAS54905.2022.9948617)
7. [Orlandic L, Thevenot J, Teijeiro T, Atienza D. A Multimodal Dataset for Automatic Edge-AI Cough Detection. Annu Int Conf IEEE Eng Med Biol Soc. 2023 Jul:2023:1-7.](https://doi.org/10.1109/embc40787.2023.10340413)
8. [Jose Angel Miranda Calero, Rodrigo Marino, Jose M. Lanza-Gutierrez, Teresa Riesgo, Mario Garcia-Valderas, and Celia Lopez-Ongil. 2018. Embedded Emotion Recognition within Cyber-Physical Systems using Physiological Signals. In 2018 Conference on Design of Circuits and Integrated Systems (DCIS). IEEE, Lyon, France, 1-6.](https://doi.org/10.1109/DCIS.2018.8681496)
9. [Saleh Baghersalimi, Alireza Amirshahi, Tomas Teijeiro, Amir Aminifar, and David Atienza. 2023. Layer-Wise Learning Framework for Efficient DNN Deployment in Biomedical Wearable Systems. In 2023 IEEE 19th International Conference on Body Sensor Networks (BSN). IEEE, Boston, MA, USA, 1-4.](https://doi.org/10.1109/BSN58485.2023.10331334)
