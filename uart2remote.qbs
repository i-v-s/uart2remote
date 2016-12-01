import qbs
import "stm32lib/Stm32Application.qbs" as Stm32Application

Project {
    name: "UART to Remote"
    minimumQbsVersion: "1.5.0"

    Stm32Application {
        deviceName: "STM32F100RBTx"
        consoleApplication: true
        type: ["application", "hex", "bin", "size", "elf", "disassembly"]

        Group {
            name : "Drivers"
            prefix : device.driverDir
            files : [
                "clock.h",
                "clock.cpp",
                "gpio.h",
                "adc.h",
                "uart.h",
                //"uart.cpp",
                "../queue.cpp"
            ]
        }

        Group {     // Properties for the produced executable
            name : "Sources"
            files : [
                "main.cpp"
            ]
        }
        cpp.defines: device.defines
        cpp.includePaths : device.includePaths

        cpp.linkerFlags : [
            "-lc",
            "-lnosys",
            "-specs=nosys.specs"
        ].concat(device.archFlags)

        cpp.cxxLanguageVersion : "c++14"

        cpp.commonCompilerFlags : [

        ].concat(device.archFlags)
    }
}
