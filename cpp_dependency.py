import os
import re


class Dependency_file:
    def __init__(self, filename):
        self.filename = filename
        self.dependencies = []
        if filename.endswith('.h'):
            self.type = 0
        elif filename.endswith('.cpp'):
            self.type = 1
        if self.type == 0:
            self.filename_opposite = filename[:-1] + 'cpp'
        else:
            self.filename_opposite = filename[:-1] + 'h'
        try:
            code = file_list[self.filename_opposite].code
            match = re.search(r'\d+', code)
            self.number = int(match.group(0))
            if self.type == 0:
                self.code = 'H' + match.group(0)
            else:
                self.code = 'CPP' + match.group(0)
        except KeyError:
            if self.type == 0:
                self.code = 'H' + str(data["num"])
            else:
                self.code = 'CPP' + str(data["num"])
            self.number = data["num"]
            data["num"] += 1


def handle_file1(filename):
    dependency_file = Dependency_file(filename)
    file_list[dependency_file.filename] = dependency_file


def handle_file2(filename, root_dir):
    dependency_file = file_list.get(filename)
    with open(os.path.join(root_dir, filename), 'r', errors='ignore') as f:
        lines = f.readlines()
        for line in lines:
            if line.startswith('#include'):
                include_file = line.split()[1].strip('"')
                try:
                    include_file = include_file.split('/')[-1]
                    dependency_file.dependencies.append(file_list[include_file].filename)
                except KeyError:
                    pass


def traverse_dir(i, root_dir):
    for root, dirs, files in os.walk(root_dir):
        for file in files:
            if file.endswith('.h') or file.endswith('.cpp'):
                if i == 1:
                    handle_file1(file)
                else:
                    handle_file2(file, root)


def generate_markdown(output):
    with open(os.path.join(output, 'cpp_dependency.md'), 'w') as f:
        f.write('# C++ Dependency\n')
        f.write('```mermaid\n')
        f.write('graph LR\n')
        f.write('classDef hClass fill:#f9f,stroke:#333,stroke-width:2px;\n')
        file_list0 = []
        for file in file_list.values():
            file_list0.append(file)
        file_list0.sort(key=lambda x: x.number)
        temp_number = 0
        for file in file_list0:
            if temp_number == file.number:
                f.write('\t')
                f.write(file.code + '(' + file.filename + ')')
            else:
                f.write('\n')
                f.write(file.code + '(' + file.filename + ')')
            if file.type == 0:
                f.write(':::hClass; ')
            else:
                f.write('; ')
            temp_number = file.number
        for file in file_list0:
            for dependency in file.dependencies:
                f.write(file.code + '-->' + file_list[dependency].code + ' ; ')
            f.write('\n')
        f.write("```")


def init_print():
    print("Welcome to use Cx330_502's cpp dependency program, the result will be saved in cpp_dependency.md and "
          "plugin mermaid is needed to preview the result.")
    print("    ___  _  _  ___  ___   ___       ___   ___  ___  ")
    print("   / __)( \\/ )(__ )(__ ) / _ \\     | __) / _ \\(__ \\ ")
    print("  ( (__  )  (  (_ \\ (_ \\( (_) )___ |__ \\( (_) )/ _/ ")
    print("   \\___)(_/\\_)(___/(___/ \\___/(___)(___/ \\___/(____)")
    print()


def end_print():
    print("The result has been saved in cpp_dependency.md, please use mermaid plugin to preview the result.")


def input_model():
    print("Please input the model you want to use:")
    print("1. input and output both in current directory")
    print("2. './data/cpp_dependency' for input and './output/cpp_dependency' for output")
    print("3. Customizing the working directory")
    while True:
        model = input("Please input 1 or 2 or 3: ")
        model = int(model)
        if model == 1:
            input_root0 = "./"
            output_root0 = "./"
            break
        elif model == 2:
            input_root0 = "./data/cpp_dependency"
            output_root0 = "./output/cpp_dependency"
            break
        elif model == 3:
            input_root0 = input("Please input the input directory: ")
            output_root0 = input("Please input the output directory: ")
            break
    os.makedirs(input_root0, exist_ok=True)
    os.makedirs(output_root0, exist_ok=True)
    return input_root0, output_root0


if __name__ == '__main__':
    init_print()
    input_root, output_root = input_model()
    file_list = {}
    data = {"num": 1}
    traverse_dir(1, input_root)
    traverse_dir(2, input_root)
    generate_markdown(output_root)
    end_print()
