#include <iostream>
#include <stdexcept>
#include <memory>
#include <fstream>

#include "driver.h"
#include "Parser\lexer.h"
#include "Parser\Parser.h"
#include "Semantic\Analysis.h"
#include "Semantic\dumpAST.h"
#include "Semantic\Translator.h"
#include "IR\QuadGenerator.h"
#include "IR\Quad.h"
#include "IR\dumpQuad.h"

int main(int argc, char* argv[])
{
    script::Driver &driver = script::Driver::instance();
    driver.ParseArguments(argc, argv);

    script::Lexer lexer;
    script::Parser parser(lexer);
    lexer.setProgram(std::string(driver.filename));
    
    script::Analysis analysis;
    std::unique_ptr<script::ASTProgram> program;
    try {
        program = std::move(parser.parse());
        program->accept(&analysis);
    }
    catch (std::runtime_error &e) {
        std::cout << e.what() << std::endl;
    }

    // Dump ast to file
    if (driver.dumpAST_)
    {
        std::string dumpFilename(driver.filename);
        dumpFilename += ".ast";
        std::fstream dumpASTFile(dumpFilename, std::ofstream::out);
        script::DumpAST dumpAST(dumpASTFile);
        program->accept(&dumpAST);
    }
    
    script::Translator translator;
    program->accept(&translator);
    script::QuadGenerator &gen = translator.codeGenerator();
    std::list<script::Quad*> codes = std::move(gen.getCode());
    if (driver.dumpIR_)
    {
        std::string dumpFilename(driver.filename);
        dumpFilename += ".ir";
        std::fstream dumpIRFile(dumpFilename, std::ofstream::out);
        script::DumpQuad dumpQuad(dumpIRFile);
        dumpQuad.dump(codes);
    }
	return 0;
}