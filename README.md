# Liszt — Piano VST3 Plugin

**Liszt** is a piano VST3 plugin built using the JUCE framework. Featuring high-quality, self-recorded piano samples, it is enhanced by a 16-channel Feedback Delay Network (FDN) reverb and versatile modulation capabilities. The result is a rich, expressive instrument tailored for modern music production.

---

## 🎹 Overview

Liszt delivers expressive piano sounds through:

- **Advanced reverb processing** using a 16-channel FDN with diffusion control
- **Dynamic velocity-sensitive ADSR envelope** Which produces multi-velocity notes even if there is only one .wav file per note
- **Dual LFO modulation** with user-defined routing
- **Real-time waveform visualization** via an integrated oscilloscope

---

## ✨ Features

### 🎼 Sampling & Expression
- **High-Quality Sampling:** Multi-velocity piano recordings for natural tone
- **Velocity-Sensitive ADSR:** Realistic dynamic shaping based on playing intensity

### 🎧 Reverb Engine
- **FDN Reverb:** 16-channel architecture for lush, spatial sound
- **Diffusion Control:** Shape the density and texture of the reverb

### 🎛️ Modulation
- **Dual LFOs:** Independent low-frequency oscillators for modulation
  - Depth control
  - Waveform shape selection
  - Target assignment (Diffusion, Decay, Predelay)

### 📊 Visualization
- **Oscilloscope:** Real-time signal monitoring directly in the plugin UI

---

## 🎚️ Controls

### 🔊 Main
- `Gain` – Master volume control  
- `Reverb Enable` – Toggle reverb on/off  

### 🌊 Reverb
- `Predelay` – Time before reverb starts (0–100 ms)  
- `Decay` – Tail length of the reverb (0.8–5.0 s)  
- `Diffusion` – Reverb reflection density  
- `Dry/Wet` – Blend between dry signal and reverb  

### 🎚️ Filtering
- `High-Pass Filter` – Remove low-end rumble (20–150 Hz)  
- `Low-Pass Filter` – Control brightness (5–16 kHz)  

### 🔁 Modulation
- `Oscillator 1` & `Oscillator 2`  
  - Depth  
  - Shape (Sine, Triangle, Square, etc.)  
  - Modulation Target (e.g., Decay, Diffusion)

---

## ⚙️ Installation

1. **Download** `Liszt.vst3` from the `builds/` folder.
2. **Copy** the file to your system’s VST3 plugin directory:
   - **Windows:** `C:\Program Files\Common Files\VST3`
   - **macOS:** `/Library/Audio/Plug-Ins/VST3/`
3. **Restart your DAW** and rescan your plugin list.

---

## 📩 Feedback

For issues or feature requests, feel free to open an issue or submit a pull request.

---

