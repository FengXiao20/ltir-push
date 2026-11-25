// ltir/src/templates.hpp —— CLI 层，非内核头文件（阶段三批准新增）
// 单一权威：22 条码的 dev/hint 双读者措辞。编译进二进制，wasm 无文件依赖。
// 第 12 条：码冻结；本表措辞可改，但必须与对应黄金 .err/.json diff 同提交。
// 占位符白名单：{line} {col} {tag} {expected_set} {got_kind}
// 禁止任何来自 subject/answer 求值的值类槽（leak_static / leak_dynamic 双扫）。
#pragma once

namespace ltir {

struct ErrorTemplate {
  const char* code;
  const char* slug;
  const char* dev;
  const char* hint;
};

inline constexpr ErrorTemplate kErrorTemplates[] = {
{"LTIR-E0001", "TAG_UNKNOWN",
 "tokenizeFile: top-level line is not one of the four tags; expected_set={expected_set}; got_kind={got_kind}",
 "第 {line} 行第 0 列出现无法识别的顶格内容。四个合法标签为 <subject domain=\"math\">、</subject>、<answer>、</answer>；请校对拼写，或给该行加缩进使其成为数学内容行。"},

{"LTIR-E0002", "TAG_INDENT",
 "tokenizeFile: tag-shaped line found after leading whitespace at {line}:{col}; routed to file layer, not expr layer",
 "第 {line} 行的标签被缩进了。请把标签移到第 0 列顶格独占一行，并顺带全面校对：标签拼写、domain 属性是否写成 domain=\"math\"（合法值仅 \"math\" / \"physics\" / \"chemistry\"）、引号是否成对、闭合标签是否带斜杠。不要改动数学内容行。"},

{"LTIR-E0003", "SUBJECT_DOMAIN_MISSING",
 "parseFile: SubjectOpen has empty domain view; attribute parse failed at {line}:{col}",
 "第 {line} 行的 <subject> 开标签缺少或写错了 domain 属性。请直接粘贴这一行：<subject domain=\"math\">；合法值仅 \"math\" / \"physics\" / \"chemistry\"，必须使用成对双引号。"},

{"LTIR-E0004", "DOMAIN_UNKNOWN",
 "parseFile: domain value not in whitelist at {line}:{col}; got_kind={got_kind}",
 "第 {line} 行的 domain 值不在白名单。合法值仅 \"math\"、\"physics\"、\"chemistry\" 三个，请改用其中之一。"},

{"LTIR-E0005", "SUBJECT_OPEN_MISSING",
 "parseFile: expected SubjectOpen at file head; got_kind={got_kind}",
 "文件缺少 <subject> 开标签块。请在文件开头顶格添加 <subject domain=\"math\"> 与 </subject>，并在两标签之间写入至少一个以空白开头的题目表达式行。"},

{"LTIR-E0006", "SUBJECT_CLOSE_MISSING",
 "parseFile: collectBody hit End before SubjectClose; expected_set={expected_set}",
 "缺少 </subject> 闭标签。请在题目内容行之后、<answer> 之前顶格补上 </subject>。"},

{"LTIR-E0007", "ANSWER_OPEN_MISSING",
 "parseFile: expected AnswerOpen got End at EOF; file grammar = subject_open body subject_close answer_open body answer_close End",
 "文件缺少 <answer> 块。请在 </subject> 之后、第 0 列顶格添加 <answer> 与 </answer>，并在两标签之间写入至少一个以空白开头的答案表达式行。不要改动 <subject> 块。"},

{"LTIR-E0008", "ANSWER_CLOSE_MISSING",
 "parseFile: collectBody hit End before AnswerClose; expected_set={expected_set}",
 "缺少 </answer> 闭标签。请在文件末尾顶格补上 </answer>。"},

{"LTIR-E0009", "BODY_EMPTY",
 "parseFile: body collected zero MathLine tokens at {line}:{col}",
 "标签之间没有任何数学内容行。请在对应开标签与闭标签之间补上至少一个以空白开头的表达式行。"},

{"LTIR-E0010", "TAG_ORDER",
 "parseFile: unexpected structural tag at {line}:{col}; tag order fixed to subject then answer",
 "标签重复或顺序颠倒。请保留一对 <subject> 块与一对 <answer> 块，且 <subject> 在前、<answer> 在后，删除多余标签。"},

{"LTIR-E0011", "TRAILING_CONTENT",
 "parseFile: MathLine remains after AnswerClose at {line}:{col}",
 "第 {line} 行是 </answer> 之后的残留内容行。请删除它，或把它移进 <answer> 块内并保持缩进。"},

{"LTIR-E0012", "UNSUPPORTED_FRAGMENT",
 "tokenizeExpr: byte at {line}:{col} outside the closed token set (digit,dot,+,-,*,/,(,)); got_kind=Invalid; LaTeX source accepted by file layer, not by expr layer",
 "第 {line} 行出现 v0.0.1 求值层不支持的 LaTeX 片段或字符。求值层仅支持数字、小数点、+ - * / 与圆括号，请改写为上述运算符的等价算术式；文件容器接受任意 LaTeX，但求值层不会重试该片段。"},

{"LTIR-E0013", "EXPR_EXPECTED",
 "parseFactor: expected expression at {line}:{col}; got_kind={got_kind}",
 "第 {line} 行第 {col} 列处缺少操作数或表达式结构不完整。请补全左操作数，或删除多余的运算符。"},

{"LTIR-E0014", "RPAREN_MISSING",
 "parseFactor: expected ')' at {line}:{col}; got_kind={got_kind}",
 "第 {line} 行的左括号没有对应的右括号。请补齐右括号，或删除多余的左括号。"},

{"LTIR-E0015", "TRAILING_TOKEN",
 "parseExprTokens: trailing token before End at {line}:{col}; got_kind={got_kind}",
 "第 {line} 行第 {col} 列之后还有多余的表达式 token。请删除尾部残留，或补上运算符把它接入表达式。"},

{"LTIR-E0016", "LITERAL_SYNTAX",
 "tokenizeExpr: bad numeric literal at {line}:{col}; grammar = digit+ [ '.' digit+ ]",
 "第 {line} 行第 {col} 列的数字字面量写法不合法。小数点前后都必须写数字，前导零不可省略，不接受 1. 或 .5 或 1.2.3。"},

{"LTIR-E0017", "LIT_RANGE",
 "parseNumber: parseDecimalToRational returned false at {line}:{col}; decimalmantissa or denominator exceeds int64 after stripping trailing zeros",
 "第 {line} 行的字面量在剥除小数尾零后仍超出 64 位有符号整数范围。请缩小字面量的位数；本解释器不会用近似值代替。"},

{"LTIR-E0018", "OVERFLOW",
 "applyBinary: checked op overflow at op_pos={line}:{col}; lhs/rhs both canonical Rational; no fallback value emitted",
 "第 {line} 行的运算使精确结果超出 64 位有符号整数范围。请缩小参与运算的字面量，或调整运算顺序；本解释器不会用近似值代替。"},

{"LTIR-E0019", "NEG_OVERFLOW",
 "applyNeg: checkedNeg failed at op_pos={line}:{col}; operand numerator is INT64_MIN",
 "第 {line} 行对边界整数取负导致溢出。请改运算结构，不要对 -9223372036854775808 这个值再取负。"},

{"LTIR-E0020", "DIV_ZERO",
 "applyBinary: Div with rhs numerator zero at op_pos={line}:{col}; no inf emitted",
 "第 {line} 行的除数为 0。请改除数或改表达式；本解释器不会返回无穷值。"},

{"LTIR-E0021", "CLI_ARGS",
 "runCli: argument vector not eval <expr> / check <file> [--json]",
 "命令行参数不合法。请使用 ltir eval 表达式，或 ltir check 文件路径；可选在最前或最后加 --json。"},

{"LTIR-E0022", "FILE_READ",
 "runCli: ifstream open failed on check input path",
 "无法读取输入文件。请检查路径拼写与文件是否存在，再重试。"},
};

inline constexpr int kErrorTemplateCount =
    sizeof(kErrorTemplates) / sizeof(kErrorTemplates[0]);

} // namespace ltir
