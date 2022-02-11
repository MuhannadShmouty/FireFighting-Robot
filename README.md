![Cover Photo of the project](https://drive.google.com/file/d/1n4yjdfQ19zOam5gqlFNfnjDna9pn8Vs7/view?usp=sharing)

# FireFighting Robot
> Smart Remote-Controlled Firefighting Robot

A fire is one of the most dangerous things that can happen in our lives and unfortunately it cannot be controlled in many cases, a single fire can destroy entire apartment blocks if left unattended for a long time, and with our constantly evolving cities firemen won’t be able to handle every fire that occurs, and that’s not taking into account that firemen usually get severely injured or burned.

The aim of this project is to create a robot to fight fires in places where humans cannot, the user controls it from a safe distance to put out fires and save lives using a remote control while getting visual feedback from a camera, using its various sensors the user can be provided with information to help assess the situation and guide their decision making.

The project has many applications from personal use to industrial and governmental use, there’s also many recommendations to improve its design for commercial manufacturing.


## Getting started

As shown in the infograph, the robot consists of two high-torque dc motors with metal gearbox, a water pump, a servo motor to direct the nozzle coming out of the pump and another to direct the onboard camera. As well as multiple sensors (temperature and ultrasonic) to provide the user with more data about the environment in which the robot is going through.
![Inphographic photo](https://drive.google.com/file/d/1wzanSIH-58Nf2ZOFWj5Fe4BqIGj8vXUL/view?usp=sharing)

A quick introduction of the minimal setup you need to get a hello world up &
running.

To apply this project, [Arduino IDE](https://www.arduino.cc/en/software) is needed in order to control the ATMEGA328P chip on the UNO and NANO development boards.

### NRF Library

In order to expect the best performance from the nRF modules, the [TMRh20 RF24 library](https://github.com/nRF24/RF24) should be installed.

## Developing

Here's a brief intro about what a developer must do in order to start developing
the project further:

```shell
git clone https://github.com/your/awesome-project.git
cd awesome-project/
packagemanager install
```

And state what happens step-by-step.

### Building

If your project needs some additional steps for the developer to build the
project after some code changes, state them here:

```shell
./configure
make
make install
```

Here again you should state what actually happens when the code above gets
executed.

### Deploying / Publishing

In case there's some step you have to take that publishes this project to a
server, this is the right time to state it.

```shell
packagemanager deploy awesome-project -s server.com -u username -p password
```

And again you'd need to tell what the previous code actually does.

## Features

What's all the bells and whistles this project can perform?
* What's the main functionality
* You can also do another thing
* If you get really randy, you can even do this

## Configuration

Here you should write what are all of the configurations a user can enter when
using the project.

#### Argument 1
Type: `String`  
Default: `'default value'`

State what an argument does and how you can use it. If needed, you can provide
an example below.

Example:
```bash
awesome-project "Some other value"  # Prints "You're nailing this readme!"
```

#### Argument 2
Type: `Number|Boolean`  
Default: 100

Copy-paste as many of these as you need.

## Contributing

When you publish something open source, one of the greatest motivations is that
anyone can just jump in and start contributing to your project.

These paragraphs are meant to welcome those kind souls to feel that they are
needed. You should state something like:

"If you'd like to contribute, please fork the repository and use a feature
branch. Pull requests are warmly welcome."

If there's anything else the developer needs to know (e.g. the code style
guide), you should link it here. If there's a lot of things to take into
consideration, it is common to separate this section to its own file called
`CONTRIBUTING.md` (or similar). If so, you should say that it exists here.

## Links

Even though this information can be found inside the project on machine-readable
format like in a .json file, it's good to include a summary of most useful
links to humans using your project. You can include links like:

- Project homepage: https://your.github.com/awesome-project/
- Repository: https://github.com/your/awesome-project/
- Issue tracker: https://github.com/your/awesome-project/issues
  - In case of sensitive bugs like security vulnerabilities, please contact
    my@email.com directly instead of using issue tracker. We value your effort
    to improve the security and privacy of this project!
- Related projects:
  - Your other project: https://github.com/your/other-project/
  - Someone else's project: https://github.com/someones/awesome-project/