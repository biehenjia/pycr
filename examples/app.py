import streamlit
import pandas as pd
import pycr

streamlit.title("Chains of Recurrences")
streamlit.markdown("Chains of Recurrences (CR's) is an effective method to evaluate functions at regular intervals. For example, a common task is to evaluate $G(x_0 +ih),\\forall 0 \\leq i \\leq s$ for some $x_0,h,s$ in the purposes of: graph plotting, signal processing, activation function approximation, etc.")
streamlit.markdown("This website serves as a demonstration of using CR's, built in `python` and `c++`.")
streamlit.header("Benchmarking")
streamlit.markdown("Input a univariate expression to benchmark evaluation time against alternatives. Input the expression to be evaluated on the right, and the associated cycle variable on the left. For example, input $x^2+x+2$ for the expression, with cycle variable $x$.")

benchmark1, benchmark2 = streamlit.columns(2)
benchexpr = benchmark1.text_input("Expression: $f(x)$")
benchcycle = benchmark2.text_input("Cycle variable: ")

if streamlit.button("Benchmark Expression"):
    data = []
    try:
        for i in range(7):
            p = [benchcycle+f",1,1,{10**i}"]
            result, time_taken = pycr.evalcr(benchexpr, p)
            compiled_time = pycr.benchmark(benchexpr, p)
            data.append({
                "Input Size": 10**i,
                "CR Time (ms)": time_taken,
                "Compiled Time (ms)": compiled_time,
                "Delta (ms)": time_taken - compiled_time,
                "Speedup" : compiled_time/time_taken if compiled_time > 0 else float('inf')
            })
    except:
        streamlit.error("Error occurred during benchmarking!")

    df = pd.DataFrame(data)
    streamlit.write("Benchmark Results:")
    streamlit.dataframe(df)
    streamlit.write(result[-1])

streamlit.header("Generation & Evaluation")
streamlit.markdown("Input an expression and parameters in the form of $x,x_0,h,i_{max}$ delimited by spaces for multivariate expressions.")
streamlit.sidebar.write("Documentation and use")

col1, col2 = streamlit.columns(2)
colexpr = col1.text_input("Expression: $f(x_1,x_2,\ldots)$")
colparam = col2.text_input("Parameters: $p^*=\{p_1,p_2,\ldots\}$")

if streamlit.button("Evaluate Expression"):
    result, time_taken = pycr.evalcr(colexpr, colparam.split())
    streamlit.write(f"Last Value: {result[-1]}, Time taken: {time_taken:.2f} ms")
    code, time_taken = pycr.crgen(colexpr, colparam.split())
    streamlit.code(code)
    print("len:", len(code), "has_nul:", "\x00" in code, "first_nul_at:", code.find("\x00"))
    if "\x00" in code:
        i = code.find("\x00")
        print(repr(code[max(0, i-20): i+20]))