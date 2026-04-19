import os
import argparse
import xml.etree.ElementTree as ET


MAX_PAYLOAD_SIZE = 254

FIELD_SIZES = {
    "uint8": 1,
    "uint16": 2,
    "uint32": 4,
    "int8": 1,
    "int16": 2,
    "int32": 4,
    "float": 4,
    "double": 8
}

BASE_PATH = os.path.dirname(os.path.abspath(__file__))


def generate_code_c(messages, enums, output_dir):
    def process_msg(msg):
        field_to_c_type = {
            "uint8": "uint8_t",
            "uint16": "uint16_t",
            "uint32": "uint32_t",
            "int8": "int8_t",
            "int16": "int16_t",
            "int32": "int32_t",
            "float": "float",
            "double": "double"
        }

        define_id = f"DATALINK_MESSAGE_ID_{msg['name']}".upper()
        define_size = f"DATALINK_MESSAGE_SIZE_{msg['name']}".upper()
        defines_string = f"#define {define_id} {msg['id']}\n#define {define_size} {msg['size']}\n"
        struct_string = f"typedef struct \n{{\n"
        pack_string_header = f"void datalink_pack_{msg['name']}(" + (f"const {msg['name']} *frame, " if msg['size'] > 0 else "") + "datalink_message_t *message);"
        unpack_string_header = f"int datalink_unpack_{msg['name']}(" + (f'{msg["name"]} *frame, ' if msg['size'] > 0 else '') + "const datalink_message_t *message);"
        pack_string_source = f"void datalink_pack_{msg['name']}(" + (f"const {msg['name']} *frame, " if msg['size'] > 0 else "") + f"datalink_message_t *message) \n{{\n    message->msg_id = {define_id};\n    message->len = {define_size};\n" + (f"    memcpy(message->payload, frame, {define_size});\n" if msg['size'] > 0 else "") + f"}}\n"
        unpack_string_source = f"int datalink_unpack_{msg['name']}(" + (f'{msg["name"]} *frame, ' if msg['size'] > 0 else '') + f"const datalink_message_t *message) \n{{\n    if (message->msg_id != {define_id} || message->len != {define_size}) return DATALINK_ERROR;\n" + (f"    memcpy(frame, message->payload, {define_size});\n" if msg['size'] > 0 else "") + f"    return DATALINK_OK;\n}}\n"

        for field in msg["fields"]:
            struct_string += f"    {field_to_c_type[field['type']]} {field['name']};" + (f" // {field['comment']}" if field['comment'] else "") + "\n"

        struct_string += f"}} {msg['name']};\n"

        return (defines_string + '\n' + (struct_string + "\n" if msg['size'] > 0 else "") + pack_string_header + "\n" + unpack_string_header + "\n\n\n", pack_string_source + "\n" + unpack_string_source + "\n")

    def process_enum(enum):
        s = ""
        s += f"typedef enum \n{{\n"

        prefix = "DATALINK_FLAGS" if enum["type"] == "flags" else "DATALINK"

        for i, value in enumerate(enum["values"]):
            s += f"    {prefix}_{value} = {(1 << i) if enum['type'] == 'flags' else i},\n"

        s += f"}} {enum['name']};\n\n"

        return s

    header_str = ""
    source_str = ""

    for enum in enums:
        header_str += process_enum(enum)
    for msg in messages:
        h, c = process_msg(msg)

        header_str += h
        source_str += c

    with open(f"{BASE_PATH}/templates/template.h", 'r') as f:
        template_h = f.read()

    with open(f"{output_dir}/datalink.h", 'w') as f:
        content = template_h.replace("// {{{DATA}}}", header_str)
        f.write(content)

    with open(f"{BASE_PATH}/templates/template.c", 'r') as f:
        template_c = f.read()

    with open(f"{output_dir}/datalink.c", 'w') as f:
        content = template_c.replace("// {{{DATA}}}", source_str).replace("#include \"template.h\"", "#include \"datalink.h\"")
        f.write(content)

    print("C code generated successfully.")


def generate_code_csharp(messages, enums, output_dir):
    def process_msg(msg):
        field_to_csharp_type = {
            "uint8": "byte",
            "uint16": "ushort",
            "uint32": "uint",
            "int8": "sbyte",
            "int16": "short",
            "int32": "int",
            "float": "float",
            "double": "double"
        }

        class_string = ""
        class_string += f"public class {msg['name']}\n{{\n"
        class_string += f"    public const int MSG_ID = {msg['id']};\n"
        class_string += f"    public const int MSG_SIZE = {msg['size']};\n"
        class_string += f"\n"

        for field in msg['fields']:
            class_string += f"    public {field_to_csharp_type[field['type']]} {field['name']} {{ get; set; }}" + (f" // {field['comment']}" if field['comment'] else "") + "\n"

        class_string += f"\n    public datalink_message Pack()\n    {{\n"

        if msg['size'] > 0:
            class_string += f"        byte[] payload = new byte[MSG_SIZE];\n\n"
            offset = 0
            for field in msg['fields']:
                class_string += f"        DataLinkMemoryUtils.Write{field['type'].capitalize()}(payload, {offset}, this.{field['name']});\n"
                offset += FIELD_SIZES[field['type']]
            class_string += f"\n        return new datalink_message(MSG_ID, payload);\n"
        else:
            class_string += f"        return new datalink_message(MSG_ID, null);\n"

        class_string += f"    }}\n"
        class_string += f"\n    public static {msg['name']} Unpack(datalink_message message)\n    {{\n"
        class_string += f"        if (message.msg_id != MSG_ID || message.len != MSG_SIZE)\n        {{\n"
        class_string += f"            return null;\n"
        class_string += f"        }}\n\n"
        class_string += f"        {msg['name']} frame = new {msg['name']}();\n\n"
        offset = 0
        for field in msg['fields']:
            class_string += f"        frame.{field['name']} = DataLinkMemoryUtils.Read{field['type'].capitalize()}(message.payload, {offset});\n"
            offset += FIELD_SIZES[field['type']]
        class_string += f"\n        return frame;\n"
        class_string += f"    }}\n"
        class_string += f"}}\n"

        return class_string + "\n"

    def process_enum(enum):
        s = ""
        s += f"public enum {enum['name']} : byte\n{{\n"

        prefix = "DATALINK_FLAGS" if enum["type"] == "flags" else "DATALINK"

        for i, value in enumerate(enum["values"]):
            s += f"    {prefix}_{value} = {(1 << i) if enum['type'] == 'flags' else i},\n"

        s += f"}};\n\n"

        return s

    txt = ""

    for enum in enums:
        txt += process_enum(enum)
    for msg in messages:
        txt += process_msg(msg)

    with open(f"{BASE_PATH}/templates/template.cs", 'r') as f:
        template_py = f.read()

    with open(f"{output_dir}/datalink.cs", 'w') as f:
        content = template_py.replace("// {{{DATA}}}", txt)
        f.write(content)

    print("C# code generated successfully.")


def generate_code_python(messages, enums, output_dir):
    def process_msg(msg):
        field_to_py_type = {
            "uint8": "int",
            "uint16": "int",
            "uint32": "int",
            "int8": "int",
            "int16": "int",
            "int32": "int",
            "float": "float",
            "double": "float"
        }

        field_to_format_char = {
            "uint8": "B",
            "uint16": "H",
            "uint32": "I",
            "int8": "b",
            "int16": "h",
            "int32": "i",
            "float": "f",
            "double": "d"
        }

        def_id_name = f"DATALINK_MESSAGE_ID_{msg['name']}".upper()
        def_size_name = f"DATALINK_MESSAGE_SIZE_{msg['name']}".upper()

        class_string = ""
        class_string += f"{def_id_name} = {msg['id']}\n"
        class_string += f"{def_size_name} = {msg['size']}\n"
        class_string += f"@dataclass\nclass {msg['name']}:\n"
        fmt = "<"
        fmt_fields = []

        for field in msg['fields']:
            class_string += f"    {field['name']}: {field_to_py_type[field['type']]}" +  (f" # {field['comment']}" if field['comment'] else "") + "\n"
            fmt += field_to_format_char[field['type']]
            fmt_fields.append("self." + field['name'])

        class_string += f"\n    def pack(self) -> datalink_message:\n"
        class_string += f"        payload = struct.pack('{fmt}', {', '.join(fmt_fields)})\n" if msg['size'] > 0 else "        payload = b''\n"
        class_string += f"        return datalink_message({def_id_name}, payload)\n"
        class_string += f"\n    @staticmethod\n    def unpack(message: datalink_message) -> '{msg['name']}':\n"
        class_string += f"        if message.msg_id != {def_id_name} or message.len != {def_size_name}:\n"
        class_string += f"            raise ValueError(f'Invalid message ID or length for unpacking {msg['name']}')\n"
        class_string += f"        fields = struct.unpack('{fmt}', message.payload)\n" if msg['size'] > 0 else ""
        class_string += f"        return {msg['name']}({', '.join(f'fields[{i}]' for i in range(len(msg['fields'])))})\n"

        return class_string + "\n\n"

    def process_enum(enum):
        s = f"class {enum['name']}:\n"

        prefix = "DATALINK_FLAGS" if enum["type"] == "flags" else "DATALINK"

        for i, value in enumerate(enum["values"]):
            s += f"    {prefix}_{value} = {(1 << i) if enum['type'] == 'flags' else i}\n"

        return s + "\n\n"

    txt = ""

    for enum in enums:
        txt += process_enum(enum)
    for msg in messages:
        txt += process_msg(msg)

    with open(f"{BASE_PATH}/templates/template.py", 'r') as f:
        template_py = f.read()

    with open(f"{output_dir}/datalink.py", 'w') as f:
        content = template_py.replace("# {{{DATA}}}", txt)
        f.write(content)

    print("Python code generated successfully.")


def parse_messages(root):
    messages = []

    if root is None:
        return messages

    for msg in root.findall("message"):
        name = msg.attrib["name"]
        msg_id = int(msg.attrib["id"])

        fields = [{
            "name": field.attrib["name"],
            "type": field.attrib["type"],
            "comment": field.text.strip() if field.text else ""
        } for field in msg.findall("field")]

        messages.append({
            "name": name,
            "id": msg_id,
            "fields": fields
        })

    return messages


def parse_enums(root):
    enums = []

    if root is None:
        return enums

    for enum in root.findall("enum"):
        name = enum.attrib["name"]
        type = enum.attrib.get("type", "normal")
        values = [val.attrib["name"] for val in enum.findall("value")]

        enums.append({
            "name": name,
            "type": type,
            "values": values
        })

    return enums


def parse_xml(xml_path: str):
    tree = ET.parse(xml_path)
    root = tree.getroot()

    return parse_messages(root.find("messages")), parse_enums(root.find("enums"))


def get_files():
    files = []

    for file in os.listdir(BASE_PATH + "/schemas"):
        if file.endswith(".xml"):
            files.append(os.path.join(BASE_PATH, "schemas", file))

    return files


def verify_msg_ids(messages):
    ids = set()

    for msg in messages:
        if msg["id"] in ids:
            raise ValueError(f"Duplicate message ID {msg['id']} found in message {msg['name']}")

        ids.add(msg["id"])


def verify_msg_names(messages):
    names = set()

    for msg in messages:
        if msg["name"] in names:
            raise ValueError(f"Duplicate message name {msg['name']} found")

        names.add(msg["name"])


def verify_msg_fields_names(messages):
    for msg in messages:
        names = set()

        for field in msg["fields"]:
            if field["name"] in names:
                raise ValueError(f"Duplicate field name {field['name']} found in message {msg['name']}")

            names.add(field["name"])


def verify_msg_sizes(messages):
    for msg in messages:
        total_size = sum(FIELD_SIZES[field['type']] for field in msg['fields'])

        if total_size > MAX_PAYLOAD_SIZE:
            raise ValueError(f"Message {msg['name']} has size {total_size} which exceeds the maximum payload size of {MAX_PAYLOAD_SIZE} bytes.")
        else:
            msg['size'] = total_size


def verify_enums_names(enums):
    names = set()

    for enum in enums:
        if enum["name"] in names:
            raise ValueError(f"Duplicate enum name {enum['name']} found")

        names.add(enum["name"])


def verify_enums_values(enums):
    for enum in enums:
        values = set()

        for value in enum["values"]:
            if value in values:
                raise ValueError(f"Duplicate enum value {value} found in enum {enum['name']}")

            values.add(value)


def reorder_messages_fields(messages):
    for msg in messages:
        msg['fields'].sort(key=lambda f: FIELD_SIZES[f['type']], reverse=True)


def create_output_dir(output_dir):
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(prog="gen.py", description="Code generator for datalink messages and enums based on XML schemas.")
    parser.add_argument("--lang", choices=["c", "csharp", "python"], help="Language to generate", required=True)
    parser.add_argument("--outdir",  help="Directory to output generated code", required=True)
    args = parser.parse_args()

    files = get_files()
    all_messages = []
    all_enums = []

    for file in files:
        print(f"Parsing {file}...")

        messages, enums = parse_xml(file)

        all_messages.extend(messages)
        all_enums.extend(enums)

    print(f"Parsed {len(all_messages)} messages and {len(all_enums)} enums.")

    verify_msg_ids(all_messages)
    verify_msg_names(all_messages)
    verify_msg_fields_names(all_messages)
    verify_msg_sizes(all_messages)
    verify_enums_names(all_enums)
    verify_enums_values(all_enums)

    reorder_messages_fields(all_messages)

    create_output_dir(args.outdir)

    if args.lang == "c":
        generate_code_c(all_messages, all_enums, args.outdir)
    elif args.lang == "csharp":
        generate_code_csharp(all_messages, all_enums, args.outdir)
    elif args.lang == "python":
        generate_code_python(all_messages, all_enums, args.outdir)
