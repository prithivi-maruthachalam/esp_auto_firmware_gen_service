import json
import argparse
import os

parser = argparse.ArgumentParser(
    prog='Code Generator',
    description='Generates custom firmware for the esp32 from json configuration files'
)
parser.add_argument('-f', '--file')
parser.add_argument('-o', '--out')
args = parser.parse_args()

if not os.path.exists(args.out):
    print(f'Creating builds source file')
    os.makedirs(args.out)


print(f'Opening file - {args.file}')
inputFile = open(args.file, 'r')
config = json.load(inputFile)

installModules = config["install"]
event_generators = config["event_generators"]

applicationHeader = open("application.h", "w")

applicationHeader.write("#ifndef APPLICATION\n")
applicationHeader.write("#define APPLICATION\n")
applicationHeader.write("\n")
applicationHeader.write("#include \"src/aws_iot.h\"\n")

for module in installModules:
    applicationHeader.write("#include "+"\"src/"+module+".h"+"\"\n")

for event_generator in event_generators:
    applicationHeader.write("void "+event_generator+"_event_Handler();\n")

applicationHeader.write("void start_application();\n")
applicationHeader.write("extern QueueHandle_t mqttData_Queue;\n")
applicationHeader.write("#endif /* APPLICATION */")

applicationHeader.close()

print(config["sequence"])

applicationSource = open("application.c", "w")

applicationSource.write("#include \"application.h\"\n\n")
applicationSource.write("void start_application()\n{\n")

for module in installModules:
    applicationSource.write("\tinstall_"+module+"();"+"\n")
for event_generator in event_generators:
    applicationSource.write("\txTaskCreate("+event_generator + "_event_Handler, " + "\"" + event_generator +
                            "_Handler\", "+event_generator+"_STACK_SIZE"+", NULL, "+event_generator+"_PRIORITY"+", NULL);\n")

applicationSource.write("}\n\n")

for module in event_generators:
    applicationSource.write("void "+module+"_event_Handler()\n")
    applicationSource.write("{\n")
    for sub_module in installModules:
        applicationSource.write("\tstr_"+sub_module +
                                "_event_t " + sub_module + "_msg;\n")

applicationSource.write("\tmqttData_str_t mqttData_e;\n")

for module in event_generators:
    applicationSource.write("\twhile(1)\n\t{\n")
    applicationSource.write(
        "\t\twhile(!xQueueReceive(q_"+module+"_event,&"+module+"_msg,portMAX_DELAY));\n")
    applicationSource.write("\t\tswitch(" + module + "_msg.msg_id)\n\t\t{\n")


sequence = config["sequence"]

for sequence_num in range(len(sequence)):
    applicationSource.write("\t\t\tcase ")
    applicationSource.write(sequence[sequence_num]["event"] + ":\n")
    for count in range(sequence[sequence_num]["number_of_actions"]):
        action_on = sequence[sequence_num]["action"][count]["action on"]
        if (action_on == 'MQTT'):
            applicationSource.write(
                "\t\t\t\tmqttData_e.data = " + module + "_msg.data" + ";\n")
            applicationSource.write(
                "\t\t\t\txQueueSend(mqttData_Queue, &mqttData_e , (TickType_t)0 );\n")
        else:
            action_name = sequence[sequence_num]["action"][count]["action"]
            applicationSource.write(
                "\t\t\t\t"+action_on + "_msg.msg_id = " + action_name + ";\n")
            applicationSource.write(
                "\t\t\t\txQueueSend(q_"+action_on + "_event, &"+action_on+"_msg, (TickType_t)0 );\n")
    applicationSource.write("\t\t\t\tbreak;\n")
applicationSource.write("\t\t\tdefault:\n\t\t\t\tbreak;\n\t\t}\n\t}\n}")

applicationSource.close()

configFile = open("config.h", "w")

configFile.write("#ifndef CONFIG\n")
configFile.write("#define CONFIG\n\n")

for module in installModules:
    configFile.write("#define "+module+"_INSTALLED\n")
configFile.write("\n#endif /* CONFIG */")
configFile.close()

# write CMakeLists
cmakefile = open("CMakeLists.txt", "w")

# write mandatory files
cmakefile.write(
    "idf_component_register(SRCS  \"src/OTA.c\" \"main.c\" \"src/app_nvs.c\" \"src/app_wifi.c\" \"src/aws_iot.c\" \"src/LED.c\" \"system.c\" \"application.c\" ")

for module in installModules:
    cmakefile.write("\"src/"+module+".c\" ")

cmakefile.write("\n\t\t\t\t\tINCLUDE_DIRS \".\")")
cmakefile.close()
