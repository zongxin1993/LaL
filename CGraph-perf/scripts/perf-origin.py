import subprocess
import time
import os
import re
import pandas as pds

cgraph_binary_path = "./lal_cg_perf"
taskflow_binary_path = "./lal_tf_perf"

xlsxFile = "./out.xlsx"


def demo(cpus, priority, path, testCount, rounds):
    cmd = "taskset -c " + cpus + " chrt -r " + str(priority) + " " + path + " " + str(testCount) + " " + str(
        rounds)
    test_process = subprocess.run(cmd, shell=True, capture_output=True, text=True)
    # 检查命令是否成功执行
    if test_process.returncode != 0:
        # 命令执行失败
        print("命令执行出错：")
        print(test_process.stderr)
        return ""
    else:
        return test_process.stdout


def calculate(listdata):
    ret = []
    listdata = list(map(float, listdata))
    # 求均值
    ret.append(pds.Series(listdata).mean())
    # 中位数
    ret.append(pds.Series(listdata).median())
    # 最大值
    ret.append(pds.Series(listdata).max())
    # 最小值
    ret.append(pds.Series(listdata).min())
    # 求标准差
    ret.append(pds.Series(listdata).std())

    return ret


def to_base_26(number):
    if number == 0:
        return 'A'

    base_26 = ''
    while number > 0:
        number -= 1  # 将数字减1以与A对应
        digit = number % 26
        base_26 = chr(digit + ord('A')) + base_26
        number //= 26

    return base_26


def parse(cgraph_out, taskflow_out):
    cgraph_array = []
    taskflow_array = []

    cg_cacu_array = []
    tf_cacu_array = []

    pattern = r"[0-9]{1,7}\.[0-9]{2}"
    cgraph_array = re.findall(pattern, cgraph_out)
    taskflow_array = re.findall(pattern, taskflow_out)

    cg_cacu_array = calculate(cgraph_array)
    tf_cacu_array = calculate(taskflow_array)

    writer = pds.ExcelWriter(xlsxFile, engine='xlsxwriter')
    writer.book.add_worksheet('原始数据')
    df1 = pds.DataFrame({'cgraph': cgraph_array,
                         'taskflow': taskflow_array})
    df1.to_excel(writer, sheet_name='原始数据', index=False)

    writer.book.add_worksheet('统计数据')

    df2 = pds.DataFrame({"": ["cgraph", "taskflow"],
                         "平均值": [cg_cacu_array[0], tf_cacu_array[0]],
                         "中位数": [cg_cacu_array[1], tf_cacu_array[1]],
                         "最大值": [cg_cacu_array[2], tf_cacu_array[2]],
                         "最小值": [cg_cacu_array[3], tf_cacu_array[3]],
                         "标准差": [cg_cacu_array[4], tf_cacu_array[4]]})
    # 写入第二个工作表
    df2.to_excel(writer, sheet_name='统计数据', index=False)
    writer.close()


cg_out = demo("13-15", 10, cgraph_binary_path, 100000, 10000)
tf_out = demo("13-15", 10, taskflow_binary_path, 100000, 10000)
parse(cg_out, tf_out)
