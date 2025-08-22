import re
import json
from pathlib import Path
import pycrlib
import functools
import time

def timing(function):
    @functools.wraps(function)
    def timewrapper(*args, **kwargs ):
        t0 = time.perf_counter()
        res = function(*args, **kwargs)
        t1 = time.perf_counter()
        print(f"finished in {1000*(t1-t0)}")
        return res
    return timewrapper

il = {}
ils = []

# ll1 stuff
# TODO : Still need to learn more about parse table in general
p = Path(__file__).parent / "ll1.json"
with open(p) as f:
    ll1 = json.load(f)

terminals = set()
for x,y in ll1.items():
    terminals.update(y.keys())
terminals.add('EOF')

lex = {
    'NUMERIC' : r'(?:\d+\.\d*|\d+|\.\d+)',
    'SIN' : r'\bsin\b',
    'COS' : r'\bcos\b',
    'TAN' : r'\btan\b',
    'EXP' : r'\bexp\b',
    'SYMBOLIC' : r'[A-Za-z]\w*',
    'OPERATOR' : r'[\+\-\*/\^!\(\),]',
    'SKIP' : r'[ \t]+'
}

master = re.compile('|'.join(f"(?P<{group}>{pattern})" for group, pattern in lex.items()))

# lexing stuff
class Token:
    def __init__(self, kind, text, pos):
        self.kind = kind
        self.text = text
        self.pos = pos 
    
    def __repr__(self):
        return f"{self.kind} Token '{self.text}' @ {self.pos}"

def tokenize(src):
    tokens = []
    for m in master.finditer(src):
        kind = m.lastgroup
        text = m.group(kind)
        pos = m.start()
        if kind == 'SKIP':
            continue 
        if kind == 'OPERATOR':
            kind = text
        tokens.append(Token(kind, text, pos))
    tokens.append(Token('EOF', '', len(src)))
    return tokens

# ast stuff
def astadd(stack,text):
    right = stack.pop()
    left = stack.pop()
        
    result = pycrlib.ASTbin(pycrlib.bt.ADD, left, right)
    #result._children = (left, right)
    stack.append(result)
    #print(type(stack[-1]))
    result._children = (left, right)

def astsub(stack,text):
    right = stack.pop()
    left = stack.pop()
    result = pycrlib.ASTbin(pycrlib.bt.SUB, left, right)
    #result._children = (left, right)

    stack.append(result)
    result._children = (left, right)

def astmul(stack,text):
    right = stack.pop()
    left = stack.pop()
    result = pycrlib.ASTbin(pycrlib.bt.MUL, left, right)
    #result._children = (left, right)

    stack.append(result)
    result._children = (left, right)

def astdiv(stack,text):
    right = stack.pop()
    left = stack.pop()
    result = pycrlib.ASTbin(pycrlib.bt.DIV, left, right)
    #result._children = (left, right)
    stack.append(result)
    result._children = (left, right)

def astexp(stack,text):
    right = stack.pop()
    left = stack.pop()
    result = pycrlib.ASTbin(pycrlib.bt.POW, left, right)
    #result._children = (left, right)

    stack.append(result)
    #print(type(stack[-1]))
    result._children = (left, right)

def astneg(stack,text):
    pass

def astfac(stack,text):
    pass

def astsin(stack,text):
    left = stack.pop()
    result = pycrlib.ASTun(pycrlib.ut.SIN, left)
    stack.append(result)
    result._children = (left)
    #stack[-1].view()

def astcos(stack,text):
    left = stack.pop()
    result = pycrlib.ASTun(pycrlib.ut.COS, left)
    stack.append(result)
    result._children = (left)

def asttan(stack,text):
    left = stack.pop()
    result = pycrlib.ASTun(pycrlib.ut.TAN, left)
    stack.append(result)
    result._children = (left)


def astexponential(stack,text):
    left = stack.pop()
    result = pycrlib.ASTun(pycrlib.ut.EXP, left)
    stack.append(result)
    result._children = (left)

def astln(stack,text):
    left = stack.pop()
    result = pycrlib.ASTun(pycrlib.ut.LN, left)
    stack.append(result)
    result._children = (left)

def astnum(stack,text):
    l = pycrlib.ASTnum(float(text))
    stack.append(l)

    #stack[-1].view()
    #print(type(stack[-1]))


def astsym(stack,text):
    s,st,q,i = il[text]
    l = pycrlib.ASTvar(i, s,st)
    stack.append(l)
    #print(type(stack[-1]))



# TODO: Modularize, strange code writing ahead...
functions = { 
    "#B+" : astadd,
    "#B-" : astsub,
    "#B*" : astmul,
    "#B/" : astdiv,
    "#B^" : astexp,
    "#U!" : astfac,
    "#U-" : astneg,
    "#N" : astnum, 
    "#SIN" : astsin,
    "#COS" : astcos,
    "#TAN" : asttan,
    "#E" : astexponential,
    "#S" : astsym,
    "#LN" : astln
}


# simple LL1 parsing
class Parser:
    def __init__(self,src):
        self.tokens = tokenize(src)
        self.kinds = [token.kind for token in self.tokens]
        # add terminating EOF
        self.stack = ['EOF', 'E']
        self.pos = 0
        self.ast = []
    
    # TODO: fix parse table logic
    def parse(self): 
        while self.stack: 
            next = self.stack.pop()
            lookahead = self.kinds[self.pos]
            # ast stuff
            if next.startswith("#"): 
                # handle node construction
                text = None
                if next in ("#N", "#S", "#SIN", "#COS", "#TAN", "#E"):
                    text = self.tokens[self.pos-1].text
                functions[next](self.ast,text)
                continue
            # handle terminal 
            if next in terminals:
                if next != lookahead: 
                    # debugging; turn off
                    raise SyntaxError(f"EXPECTED {next} GOT {lookahead}!!!")
                self.pos +=1
                continue
            # nonterminal
            production = ll1[next].get(lookahead)
            if production is None:
                # debugging; turn off
                raise SyntaxError(f"NO RULE FOR {next} FOR {lookahead}")
            for symbol in reversed(production):
                self.stack.append(symbol)
        #print("Parsing done!")
        #self.ast[0].view()

        return self.ast[0]

def delim(s):
    tokens = re.findall(r'\d+\.\d+|\d+|[A-Za-z]+', s)
    return ','.join(tokens)

def evalcr(expr, *params):
    ils = []
    for p in params[0]:
        p = delim(p)
        l = p.split(',')
        il[l[0]] = list(map(float,l[1:])) + [len(il)]
        ils.append(int(l[3])) 
    ast = Parser(expr)
    cr = ast.parse()
    cr.crinit(ils)

    s = time.perf_counter()
    result =  cr.creval()

    return result,1000*(time.perf_counter() - s)

def crgen(expr, *params):
    ils = []
    for p in params[0]:
        p = delim(p)
        l = p.split(',')
        if not l[0] in il:
            il[l[0]] = list(map(float,l[1:])) + [len(il)]
        ils.append(int(l[3])) 
    ast = Parser(expr)
    cr = ast.parse()
    cr.crinit(ils)
    s = time.perf_counter()
    x = cr.crgen(),1000*(time.perf_counter() - s)
    
    return x

def npgen(expr, *params):
    ils = []
    for p in params[0]:
        p = delim(p)
        l = p.split(',')
        il[l[0]] = list(map(float,l[1:])) + [len(il)]
        ils.append(int(l[3])) 
    ast = Parser(expr)
    cr = ast.parse()
    s = time.perf_counter()
    x = cr.npgen(),1000*(time.perf_counter() - s)
    
    return x


def mtp(expr):
    expr = re.sub(r'\^', r'**', expr)
    expr = re.sub(r'(?<![\w.])(\d+)(?![\w.])', r'\1.0', expr)

    return expr

def naiveinit(expr,dim_specs):
    expr = mtp(expr)
    lines = []
    indent = ""
    prod = 1
    for spec in dim_specs:
        q = spec.split(",")[-1]
        prod *= int(q)
    lines.append(f"results = [0] * {prod}\ni=0")
    for spec in dim_specs:
        var, start, step, stop = map(str.strip, spec.split(","))
        lines.append(f"{indent}for _{var} in range({start}, {stop}, {step}):")
        indent += "    "
        lines.append(f"{indent}{var} = float(_{var})")
        
    lines.append(f"{indent}results[i] = {expr}")
    lines.append(f"{indent}i+=1")

    return "\n".join(lines)