# GitHub Copilot Instructions

This file provides context and guidelines for GitHub Copilot when working in this repository.

## Project Overview

<!-- Describe your project here -->
This project will take the existing protocols and methods for implementing a bluetooth controller for the df-pong game. The current version uses local .h files to implement the methods, but this project will convert it to a standar Arduino library that can be hosted through the library manager. 

The existing version is for an arduino nano 33 iot. The library should be created to work seamlessly with:
- Arduino Nano 33 IOT
- Arduino Nano BLE Sense
- Arduino R4 Wifi
- ESP32

## Code Style & Conventions

<!-- Add your coding standards here -->
- Carefully follow the requirements of the Arbuino Library manager to structure the library and associated files
- The goal for the library is to create data structures that are as beginner friendly as possible.
- - Avoid complex constructors that require &variables or pointers
- - Clearly comment the library file and examples

## Architecture & Patterns

<!-- Describe your project's architecture -->
- Document key architectural decisions here
- List important design patterns used
- Follow the Arduino library guidlines

## Additional Context

<!-- Add any other relevant information for Copilot -->
- Existing examples that implement the code: https://github.com/DigitalFuturesOCADU/df-pong/tree/main/controller
- Full Game repo: https://github.com/DigitalFuturesOCADU/df-pong
- I am the owner of the repo, so it is also available locally. It is at the same level as this folder in the local /Github/ directory.
