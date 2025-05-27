# TR-123e

**TR-123e** is a monophonic embedded bass synthesiser inspired by the iconic Moog ladder filter architecture, conceived and developed by Timothy Paul Read (under the supervision of Dr Eleonora Oreggia) using the Bela real-time audio platform, as the final project of the undergraduate degree BMus/BSc Electronic Music with Computing & Technology, at Goldsmiths, University of London. This repository contains the complete C++ source code. Full interactive online documentation is also available. Check the link below.

## Overview

TR-123e translates a sophisticated Max/MSP gen\~ patch into a modular embedded synthesiser using Bela. It features:

* Zero Delay Feedback (ZDF) four-pole Moog ladder filter
* Real-time MIDI input processing with portamento
* Dynamic ADSR envelope modulation
* An expressive eight-parameter hardware interface
* Audiovisual integration with TouchDesigner

## Project Motivation

This project addresses the challenge of maintaining the sonic expressiveness and responsiveness of software-based synthesis when translated into hardware. By implementing efficient DSP techniques and integrating performance-focused controls, TR-123e delivers a highly responsive, expressive instrument suitable for live electronic music performance and audiovisual installations.

## Key Technical Features

* **Real-Time DSP Optimization**: Achieves stable, low-latency (\~1ms) performance on ARM architecture.
* **Modular C++ Architecture**: Clearly structured, modular classes for easy modification and expansion.
* **Comparative Filter Implementation**: Includes systematic comparisons of filter implementations, emphasizing the ZDF approach.
* **Robust MIDI Handling**: Accurate timing and portamento processing for seamless pitch transitions.
* **Audiovisual Interaction**: Integrated with TouchDesigner to visually represent sound parameters in real-time.

  ## Repository Contents

* `.` – TR-123e Synthesiser C++ codebase
* `DEV/` – Comprehensive filter examples, technical breakdowns, and render setups
* `schematics/` – Hardware interface design and wiring diagrams

## Hardware Requirements

* Bela platform (ARM-based embedded audio processor)
* MIDI controller (tested with Arturia MiniLab 3)
* Breadboard or custom control interface with potentiometers

## Installation and Setup

1. Clone this repository:

```bash
git clone https://github.com/YourUsername/TR123e.git
```

2. Deploy the project onto your Bela board using the Bela IDE or terminal:

```bash
scp -r TR123e root@bela.local:/root/Bela/projects/
```

3. Connect your hardware interface and MIDI controller according to schematics provided in `schematics/`.

## Usage

* **Run TR-123e** from the Bela IDE.
* **Adjust parameters** using the hardware potentiometers for immediate sonic control.
* **Connect MIDI controllers** for extended expressive control and interaction.

## Documentation

For detailed technical insights and theoretical background, check the documentation here : http://www.gaialive.com/TR123e/html/

## License

TR-123e is distributed under the MIT License. See `LICENSE` for details.

## Contributing

Contributions and enhancements to TR-123e are welcome. Please submit issues or pull requests for review.

## Author

**Timothy Paul Read**
Goldsmiths, University of London – Electronic Music, Computing & Technology, 2025. All rights reserved.

---

**TR-123e**
an exploration of technology, cognition, and creative expression, positioned at the intersection of digital precision and analogue grit.
