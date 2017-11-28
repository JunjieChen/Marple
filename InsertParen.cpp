// insert functions 
#include <sstream>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>

#include "clang/AST/AST.h"
#include "clang/Lex/Lexer.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/ASTConsumers.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "llvm/Support/raw_ostream.h"

using namespace clang;
using namespace clang::driver;
using namespace clang::tooling;
using namespace std;

static llvm::cl::OptionCategory ToolingSampleCategory("Tooling Insertion");
static int lastLine = 0;
class CodeLib{
public:

  int getCurrentPos(){
    return fileLineNo;
  }

  bool isEof(){
    return (*codelibin).eof();
  }

  CodeLib(string file){
      loadFile(file);
  }

  CodeLib(){
    ;
  }

  string readLine(){
    if(isEof())
      return "";
    string st;
    (*codelibin)>>st;
    return st;
  }

private:
  string codelibfile;
  int fileLineNo;
  ifstream* codelibin;

  void loadFile(string file){
    fileLineNo=0;
    codelibfile=file;
    codelibin=new ifstream(codelibfile);
  }
};

CodeLib cl;
bool codelibmark;
vector<bool> linevismark;

// By implementing RecursiveASTVisitor, we can specify which AST nodes
// we're interested in by overriding relevant methods.
class MyASTVisitor : public RecursiveASTVisitor<MyASTVisitor> {
public:
  MyASTVisitor(Rewriter &R) : TheRewriter(R) {}

  int lineof(SourceLocation sl){
        return TheRewriter.getSourceMgr().getSpellingLineNumber(sl, NULL);
  }

  bool HasCompStmt(Stmt *s){
    for(Stmt::child_iterator ci=s->child_begin();ci!=s->child_end();ci++)
        if((*ci)!=NULL && isa<CompoundStmt>(*ci))
                return true;
    return false;
  }

  bool insBofStmt(Stmt *s){
      Stmt *lastchild=s;

      //TheRewriter.InsertTextBefore(s->getLocStart(),s->getStmtClassName());
      //TheRewriter.InsertTextAfter(s->getLocEnd(),"#");

      if(lastchild==NULL)
        return false;
      //TheRewriter.InsertTextAfterToken(lastchild->getLocEnd().getLocWithOffset(1),"\n}"); //分号之后插入.
      //insert right brace 
      SourceLocation endOfToken = clang::Lexer::findLocationAfterToken(
        lastchild->getLocEnd(), tok::semi, TheRewriter.getSourceMgr(), TheRewriter.getLangOpts(), false);
      SourceLocation lastRBrace = clang::Lexer::findLocationAfterToken(
        lastchild->getLocEnd(), tok::r_brace, TheRewriter.getSourceMgr(), TheRewriter.getLangOpts(), false);

      //可能出现的情况if()while(){}；if()while(){;} if()while(){} if()while();
      if(!endOfToken.isValid())
        return false;

      TheRewriter.InsertTextBefore(lastchild->getLocStart(),"{\n");
      TheRewriter.InsertTextAfterToken(endOfToken,"\n}"); //分号之后插入.
      return true;
  }

  bool insB(Stmt *s){
      Stmt *lastchild;
      for(Stmt::child_iterator ci=s->child_begin();ci!=s->child_end();ci++)
            lastchild=(*ci);

      insBofStmt(lastchild);
      return true;
  }

  void tryaddbrace(Stmt *s){
		if(s==NULL)
        return;
		if(HasCompStmt(s))
        {
         //   TheRewriter.InsertTextBefore(s->getLocStart(),"#");//cl.readLine());
         //   TheRewriter.InsertTextAfter(s->getLocEnd(),"$");
        }
        else
        {
          insB(s);
          // 如果不含有CompStmt
          // 不含有CompStmt不一定body只有一行。需要确定界限。ls
        }
  }
  
  void insert(Stmt * stmt) {
		if (stmt == NULL)
				return;
			if (isa<CompoundStmt>(stmt))
				return;
			SourceLocation endOfToken = clang::Lexer::findLocationAfterToken(
			stmt->getLocEnd(), tok::semi, TheRewriter.getSourceMgr(), TheRewriter.getLangOpts(), false);
			TheRewriter.InsertTextBefore(stmt->getLocStart(), "{\n");
			TheRewriter.InsertTextAfterToken(endOfToken, "\n}\n");
  }
  
  bool VisitStmt(Stmt *s) {
    if(s==NULL)
      return true;
	// if(TheRewriter.getSourceMgr().getFilename(s->getLocStart()).str() == 	"/home/seitest/chenjj/compiler/ours/empirical_study/verification/csmith-2.2.0/runtime/test0.c")  
	// {
	string lines;
	//cout<<"---"<<endl;	
	//ifstream fin("/home/seitest/huwx/csmith-2.2.0/runtime/test.c.gcov");//~~~
	int nowLine = lineof(s->getLocStart());
	bool isExecuted = true;
	
	//cout<<startLine<<" "<<endLine<<endl;
	if(nowLine > lastLine)
	{
		/*while(true)
		{
			getline(fin, lines);
			int first_colon = lines.find_first_of(':', 0);
			int first_not_space = lines.find_first_not_of(' ', 0);
			int second_colon = lines.find_first_of(':', first_colon + 1);
			int second_not_space = lines.find_first_not_of(' ', first_colon + 1);
		
			string executed = lines.substr(first_not_space, first_colon - first_not_space);
			string lineNum = lines.substr(second_not_space, second_colon - second_not_space);
			if(atoi(lineNum.c_str()) == nowLine)
			{
				if(executed == "#####")
					isExecuted = false;
				else
					isExecuted = true;
		
				break;
			}
		}
		fin.close();*/
		if(isExecuted)//该语句是执行过的语句
		{
			if(isa<CaseStmt>(s))
				insBofStmt(((CaseStmt*)s)->getSubStmt());
			
			if(isa<LabelStmt>(s))
				insBofStmt(((LabelStmt*)s)->getSubStmt());
		
			// 对于for & while ,如果缺少花括叿则添加花括号.
			if(isa<WhileStmt>(s) || isa<ForStmt>(s))
				tryaddbrace(s);

			if(isa<IfStmt>(s))//if 语句
			{
				Stmt *thenstmt,*elsestmt;
				thenstmt=((IfStmt*)s)->getThen();
				elsestmt=((IfStmt*)s)->getElse();

				if(thenstmt!=NULL && !isa<CompoundStmt>(thenstmt))
					insBofStmt(thenstmt);
				if(elsestmt!=NULL && !isa<CompoundStmt>(elsestmt))
					insBofStmt(elsestmt);
			}

			// 根据行号来判断是否可以插兿
			if(!codelibmark)
			{
				lastLine = nowLine;
				return true;
			}

			//int line_no=TheRewriter.getSourceMgr().getSpellingLineNumber(s->getLocStart(), NULL);
			int line_no=lineof(s->getLocStart());
			if(linevismark.size()<line_no+1)
				linevismark.resize(line_no+1,false);

			if(linevismark[line_no])
			{
				lastLine = nowLine;
				return true;
			}

			//特殊行处琿
			if(!isa<Expr>(s))
			{
				linevismark[line_no]=true;
				if(isa<DoStmt>(s)){
					//int doendline=TheRewriter.getSourceMgr().getSpellingLineNumber(s->getLocEnd(), NULL);
					int doendline=lineof(s->getLocEnd());
					if(linevismark.size()<doendline+1)
						linevismark.resize(doendline+1,false);
					linevismark[doendline]=true;
				}
				lastLine = nowLine;
				return true;
			}


			linevismark[line_no]=true;
	
			lastLine = nowLine;
			return true;
		}
  
		else
		{
			if(isa<CaseStmt>(s))
				insBofStmt(((CaseStmt*)s)->getSubStmt());
			
			if(isa<LabelStmt>(s))
				insBofStmt(((LabelStmt*)s)->getSubStmt());
			// 对于for & while ,如果缺少花括叿则添加花括号.
			if(isa<WhileStmt>(s) || isa<ForStmt>(s))
				tryaddbrace(s);

			if(isa<IfStmt>(s))//if 语句
			{
				Stmt *thenstmt,*elsestmt;
				thenstmt=((IfStmt*)s)->getThen();
				elsestmt=((IfStmt*)s)->getElse();

				if(thenstmt!=NULL && !isa<CompoundStmt>(thenstmt))
					insBofStmt(thenstmt);
				if(elsestmt!=NULL && !isa<CompoundStmt>(elsestmt))
					insBofStmt(elsestmt);
			}

			// 根据行号来判断是否可以插兿
			if(!codelibmark)
			{
				lastLine = nowLine;
				return true;
			}

			//int line_no=TheRewriter.getSourceMgr().getSpellingLineNumber(s->getLocStart(), NULL);
			int line_no=lineof(s->getLocStart());
			if(linevismark.size()<line_no+1)
				linevismark.resize(line_no+1,false);

			if(linevismark[line_no])
			{
				lastLine = nowLine;
				return true;
			}

			//特殊行处琿
			if(!isa<Expr>(s))
			{
				linevismark[line_no]=true;
				if(isa<DoStmt>(s)){
					//int doendline=TheRewriter.getSourceMgr().getSpellingLineNumber(s->getLocEnd(), NULL);
					int doendline=lineof(s->getLocEnd());
					if(linevismark.size()<doendline+1)
						linevismark.resize(doendline+1,false);
					linevismark[doendline]=true;
				}
				lastLine = nowLine;
				return true;
			}

			srand( (unsigned)time( NULL ) );
			int r = (1 + rand() % 10);
    
			if(r < 2 && !cl.isEof())
				TheRewriter.InsertTextAfter(s->getLocStart(),cl.readLine());

			linevismark[line_no]=true;
		
			
			lastLine = nowLine;
			return true;
		}
		lastLine = nowLine;
	}
	
		
	// }
	return true;
    
  }


private:
  Rewriter &TheRewriter;
};

// Implementation of the ASTConsumer interface for reading an AST produced
// by the Clang parser.
class MyASTConsumer : public ASTConsumer {
public:
  MyASTConsumer(Rewriter &R) : Visitor(R) {}

  // Override the method that gets called for each parsed top-level
  // declaration.
  bool HandleTopLevelDecl(DeclGroupRef DR) override {
    for (DeclGroupRef::iterator b = DR.begin(), e = DR.end(); b != e; ++b) {
      // Traverse the declaration using our AST visitor.
      Visitor.TraverseDecl(*b);
      //(*b)->dump();
    }
    return true;
  }

private:
  MyASTVisitor Visitor;
};

// For each source file provided to the tool, a new FrontendAction is created.
class MyFrontendAction : public ASTFrontendAction {
public:
  MyFrontendAction() {}
  
    int CopyFile(char *SourceFile,char *NewFile) 
  {  
	ifstream in; 
	ofstream out;  
	in.open(SourceFile,ios::binary);//???τ? 
	if(in.fail())//???τ??? 
	{     
		cout<<"Error 1: Fail to open the source file."<<endl;    
		in.close();    
		out.close();    
		return 0; 
	}  
	out.open(NewFile,ios::binary);//????τ?  
	if(out.fail())//??τ??? 
	{     
		cout<<"Error 2: Fail to create the new file."<<endl;    
		out.close();    
		in.close();    
		return 0; 
	}  
	else//??τ? 
	{     
		out<<in.rdbuf();    
		out.close();    
		in.close();    
		return 1; 
	} 
  }
  
  void EndSourceFileAction() override {
    SourceManager &SM = TheRewriter.getSourceMgr();
   /* llvm::errs() << "** EndSourceFileAction for: "
                 << SM.getFileEntryForID(SM.getMainFileID())->getName() << "\n";
    */
    // Now emit the rewritten buffer.
	// CopyFile("fail.c", "/home/seitest/chenjj/compiler/ours/empirical_study/verification/csmith-2.2.0/runtime/variant.c");
	//rename("/home/seitest/chenjj/compiler/test/csmith-2.2.0/runtime/test0.c", "/home/seitest/chenjj/compiler/test/csmith-2.2.0/runtime/variant.c");
    //TheRewriter.getEditBuffer(SM.getMainFileID()).write(llvm::outs());
	TheRewriter.overwriteChangedFiles();
	// rename("/home/seitest/chenjj/compiler/ours/empirical_study/verification/csmith-2.2.0/runtime/test0.c", "/home/seitest/chenjj/compiler/ours/empirical_study/verification/csmith-2.2.0/runtime/test1.c");
	// rename("/home/seitest/chenjj/compiler/ours/empirical_study/verification/csmith-2.2.0/runtime/variant.c", "/home/seitest/chenjj/compiler/ours/empirical_study/verification/csmith-2.2.0/runtime/test0.c");
  }

  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI,
                                                 StringRef file) override {
   /*  llvm::errs() << "** Creating AST consumer for: " << file << "\n"*/
    TheRewriter.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
    return llvm::make_unique<MyASTConsumer>(TheRewriter);
  }

private:
  Rewriter TheRewriter;
};

int main(int argc, const char **argv) {
  //Load config file
  ifstream cfgfile("inscfg.txt");
  string codelibpath; //codelib为code library 的目彿
  cfgfile>>codelibpath;

  //
  codelibmark=true;
  cl=CodeLib(codelibpath);

  CommonOptionsParser op(argc, argv, ToolingSampleCategory);
  ClangTool Tool(op.getCompilations(), op.getSourcePathList());

  return Tool.run(newFrontendActionFactory<MyFrontendAction>().get());
}
