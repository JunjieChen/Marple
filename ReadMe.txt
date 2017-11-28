Welcome to the homepage of Marple

File "mutationRules.txt" lists all mutation rules used by Marple, whose totle number is 132.

File "addQualifier.cpp" implements the mutation rules that are related to "add qualifier". Input: qualifier name

File "addRepModifier.cpp" implements the mutation rules that are related to "add modifier". Input: modifier name

File "remModifierQualifier.cpp" implements the mutation rules that are related to "remove modifier" and "remove qualifier". Input: modifier/qualifier name

File "repBinaryOp.cpp" implements the mutation rules that are related to "replace binary operator". Inputs: old Operator, new Operator

File "RepIntConstant.cpp" implements the mutation rules that are related to "replace constant". Input: used operation for constant.

File "repRemUnaryOp.cpp" implements the mutation rules that are related to "replace/remove unary operator". Inputs: old Operator, new Operator ("delete" represents removing it)

File "repVarSameScope.cpp" implements the mutation rules that are related to "replace variables under the same scope"

File "searchMutate.py" implements the guided search process to construct a set of passing test programs

File "collectcov.py" implements the collection of coverage of a failing test program

File "rankingFile.py" implements the aggregating-based ranking for bug localization