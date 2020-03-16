/*
 * This file is part of IIC-JKU QFR library which is released under the MIT license.
 * See file README.md or go to http://iic.jku.at/eda/research/quantum/ for more information.
 */

#include "qasm_parser/Parser.hpp"


 namespace qasm {
     
    /***
     * Private Methods
     ***/
    Parser::Expr* Parser::Exponentiation() {
        Expr* x;
        if(sym == Token::Kind::real) {
            scan();
            return new Expr(Expr::Kind::number, t.valReal);
        } else if (sym == Token::Kind::nninteger) {
            scan();
            return new Expr(Expr::Kind::number, t.val);
        } else if (sym == Token::Kind::pi) {
            scan();
            return new Expr(Expr::Kind::number, PI);
        } else if (sym == Token::Kind::identifier) {
            scan();
            return new Expr(Expr::Kind::id, 0, nullptr, nullptr, t.str);
        } else if (sym == Token::Kind::lpar) {
            scan();
            x = Exp();
            check(Token::Kind::rpar);
            return x;
        } else if (unaryops.find(sym) != unaryops.end()) {
            auto op = sym;
            scan();
            check(Token::Kind::lpar);
            x = Exp();
            check(Token::Kind::rpar);
            if (x->kind == Expr::Kind::number) {
                if (op == Token::Kind::sin) {
                    x->num = std::sin(x->num);
                } else if (op == Token::Kind::cos) {
                    x->num = std::cos(x->num);
                } else if (op == Token::Kind::tan) {
                    x->num = std::tan(x->num);
                } else if (op == Token::Kind::exp) {
                    x->num = std::exp(x->num);
                } else if (op == Token::Kind::ln) {
                    x->num = std::log(x->num);
                } else if (op == Token::Kind::sqrt) {
                    x->num = std::sqrt(x->num);
                }
                return x;
            } else {
                if (op == Token::Kind::sin) {
                    return new Expr(Expr::Kind::sin, 0, x);
                } else if (op == Token::Kind::cos) {
                    return new Expr(Expr::Kind::cos, 0, x);
                } else if (op == Token::Kind::tan) {
                    return new Expr(Expr::Kind::tan, 0, x);
                } else if (op == Token::Kind::exp) {
                    return new Expr(Expr::Kind::exp, 0, x);
                } else if (op == Token::Kind::ln) {
                    return new Expr(Expr::Kind::ln, 0, x);
                } else if (op == Token::Kind::sqrt) {
                    return new Expr(Expr::Kind::sqrt, 0, x);
                }
            }
        } else {
        	error("Invalid Expression");
        }

        return nullptr;
    }

    Parser::Expr* Parser::Factor() {
        Expr* x;
        Expr* y;
        x = Exponentiation();
        while (sym == Token::Kind::power) {
            scan();
            y = Exponentiation();
            if ( x->kind == Expr::Kind::number && y->kind == Expr::Kind::number) {
                x->num = std::pow(x->num, y->num);
                delete y;
            } else {
                x = new Expr(Expr::Kind::power, 0, x, y);
            }
        }
        return x;
    }

    Parser::Expr* Parser::Term() {
        Expr* x = Factor();
        Expr* y;

        while(sym == Token::Kind::times || sym == Token::Kind::div) {
            auto op = sym;
            scan();
            y = Factor();
            if (op == Token::Kind::times) {
                if (x->kind == Expr::Kind::number && y->kind == Expr::Kind::number) {
                    x->num = x->num * y->num;
                    delete y;
                } else {
                    x = new Expr(Expr::Kind::times, 0, x, y);
                }
            } else {
                if (x->kind == Expr::Kind::number && y->kind == Expr::Kind::number) {
                    x->num = x->num / y->num;
                    delete y;
                } else {
                    x = new Expr(Expr::Kind::div, 0, x, y);
                }
            }
        }
        return x;
    }

    Parser::Expr* Parser::Exp() {
        Expr* x;
        Expr* y;
        if (sym == Token::Kind::minus) {
            scan();
            x = Term();
            if (x->kind == Expr::Kind::number)
                x->num= -x->num;
            else
                x = new Expr(Expr::Kind::sign, 0, x);
        } else {
            x = Term();
        }

        while(sym == Token::Kind::plus || sym == Token::Kind::minus) {
            auto op = sym;
            scan();
            y = Term();
            if (op == Token::Kind::plus) {
                if (x->kind == Expr::Kind::number && y->kind == Expr::Kind::number)
                    x->num += y->num;
                else
                    x = new Expr(Expr::Kind::plus, 0, x, y);
            } else {
                if (x->kind == Expr::Kind::number && y->kind == Expr::Kind::number)
                    x->num -= y->num;
                else
                    x = new Expr(Expr::Kind::minus, 0, x, y);
            }
        }
        return x;
    }

    Parser::Expr* Parser::RewriteExpr(Expr *expr, std::map<std::string, Expr *>& exprMap) {
        if (expr == nullptr) return nullptr;
        Expr *op1 = RewriteExpr(expr->op1, exprMap);
        Expr *op2 = RewriteExpr(expr->op2, exprMap);

        if (expr->kind == Expr::Kind::number) {
            return new Expr(expr->kind, expr->num, op1, op2, expr->id);
        } else if (expr->kind == Expr::Kind::plus) {
            if (op1->kind == Expr::Kind::number && op2->kind == Expr::Kind::number) {
                op1->num = op1->num + op2->num;
                delete op2;
                return op1;
            }
        } else if (expr->kind == Expr::Kind::minus) {
            if (op1->kind == Expr::Kind::number && op2->kind == Expr::Kind::number) {
                op1->num = op1->num - op2->num;
                delete op2;
                return op1;
            }
        } else if (expr->kind == Expr::Kind::sign) {
            if (op1->kind == Expr::Kind::number) {
                op1->num = -op1->num;
                return op1;
            }
        } else if (expr->kind == Expr::Kind::times) {
            if (op1->kind == Expr::Kind::number && op2->kind == Expr::Kind::number) {
                op1->num = op1->num * op2->num;
                delete op2;
                return op1;
            }
        } else if (expr->kind == Expr::Kind::div) {
            if (op1->kind == Expr::Kind::number && op2->kind == Expr::Kind::number) {
                op1->num = op1->num / op2->num;
                delete op2;
                return op1;
            }
        } else if (expr->kind == Expr::Kind::power) {
            if (op1->kind == Expr::Kind::number && op2->kind == Expr::Kind::number) {
                op1->num = std::pow(op1->num, op2->num);
                delete op2;
                return op1;
            }
        } else if (expr->kind == Expr::Kind::sin) {
            if (op1->kind == Expr::Kind::number) {
                op1->num = std::sin(op1->num);
                return op1;
            }
        } else if (expr->kind == Expr::Kind::cos) {
            if (op1->kind == Expr::Kind::number) {
                op1->num = std::cos(op1->num);
                return op1;
            }
        } else if (expr->kind == Expr::Kind::tan) {
            if (op1->kind == Expr::Kind::number) {
                op1->num = std::tan(op1->num);
                return op1;
            }
        } else if (expr->kind == Expr::Kind::exp) {
            if (op1->kind == Expr::Kind::number) {
                op1->num = std::exp(op1->num);
                return op1;
            }
        } else if (expr->kind == Expr::Kind::ln) {
            if (op1->kind == Expr::Kind::number) {
                op1->num = std::log(op1->num);
                return op1;
            }
        } else if (expr->kind == Expr::Kind::sqrt) {
            if (op1->kind == Expr::Kind::number) {
                op1->num = std::sqrt(op1->num);
                return op1;
            }
        } else if (expr->kind == Expr::Kind::id) {
            return new Expr(*exprMap[expr->id]);
        }

        return new Expr(expr->kind, expr->num, op1, op2, expr->id);
    }




    /***
     * Public Methods
     ***/
    void Parser::scan() {
        t = la;
        la = scanner->next();
        sym = la.kind;
    }

    void Parser::check(Token::Kind expected) {
        if (sym == expected)
            scan();
        else
	        error("ERROR while parsing QASM file: expected '" + qasm::KindNames[expected] + "' but found '" + qasm::KindNames[sym] + "' in line " + std::to_string(la.line) + ", column " + std::to_string(la.col));
    }


    std::pair<unsigned short , unsigned short> Parser::ArgumentQreg() {
        check(Token::Kind::identifier);
        std::string s = t.str;
        if (qregs.find(s) == qregs.end())
	        error("Argument is not a qreg: " + s);

        if (sym == Token::Kind::lbrack) {
            scan();
            check(Token::Kind::nninteger);
            auto offset = static_cast<unsigned short>(t.val);
            check(Token::Kind::rbrack);
            return std::make_pair(qregs[s].first + offset, 1);
        }
        return std::make_pair(qregs[s].first, qregs[s].second);
    }

    std::pair<unsigned short, unsigned short> Parser::ArgumentCreg() {
        check(Token::Kind::identifier);
        std::string s = t.str;
        if (cregs.find(s) == cregs.end())
	        error("Argument is not a creg: " + s);

        if (sym == Token::Kind::lbrack) {
            scan();
            check(Token::Kind::nninteger);
            auto offset = static_cast<unsigned short>(t.val);
            check(Token::Kind::rbrack);
            return std::make_pair(cregs[s].first+offset, 1);
        }

        return std::make_pair(cregs[s].first, cregs[s].second);
    }

    void Parser::ExpList(std::vector<Expr*>& expressions) {
        expressions.emplace_back(Exp());
        while(sym == Token::Kind::comma) {
            scan();
            expressions.emplace_back(Exp());
        }
    }

    void Parser::ArgList(std::vector<std::pair<unsigned short, unsigned short>>& arguments) {
        arguments.emplace_back(ArgumentQreg());
        while(sym == Token::Kind::comma) {
            scan();
            arguments.emplace_back(ArgumentQreg());
        }
    }

    void Parser::IdList(std::vector<std::string>& identifiers) {
        check(Token::Kind::identifier);
        identifiers.emplace_back(t.str);
        while (sym == Token::Kind::comma) {
            scan();
            check(Token::Kind::identifier);
            identifiers.emplace_back(t.str);
        }
    }

    std::unique_ptr<qc::Operation> Parser::Gate() {
        if (sym == Token::Kind::ugate) {
            scan();
            check(Token::Kind::lpar);
            std::unique_ptr<Expr> theta(Exp());
            check(Token::Kind::comma);
            std::unique_ptr<Expr> phi(Exp());
            check(Token::Kind::comma);
            std::unique_ptr<Expr> lambda(Exp());
            check(Token::Kind::rpar);
            auto target = ArgumentQreg();
            check(Token::Kind::semicolon);

            if (target.second == 1) {
                return std::make_unique<qc::StandardOperation>(nqubits, target.first, qc::U3, lambda->num, phi->num, theta->num);
            }

            // TODO: multiple targets could be useful here
            auto gate = qc::CompoundOperation(nqubits);
            for (unsigned short i = 0; i < target.second; ++i) {
                gate.emplace_back<qc::StandardOperation>(nqubits, target.first + i, qc::U3, lambda->num, phi->num, theta->num);
            }
            return std::make_unique<qc::CompoundOperation>(gate);
        } else if (sym == Token::Kind::swap) {
        	scan();
        	auto first_target = ArgumentQreg();
	        check(Token::Kind::comma);
			auto second_target = ArgumentQreg();
	        check(Token::Kind::semicolon);

	        // return corresponding operation
	        if (first_target.second == 1 && second_target.second == 1) {
		        return std::make_unique<qc::StandardOperation>(nqubits, std::vector<qc::Control>{}, first_target.first, second_target.first, qc::SWAP);
	        } else {
		        error("SWAP for whole qubit registers not yet implemented");
	        }
        } else if (sym == Token::Kind::cxgate) {
            scan();
            auto control = ArgumentQreg();
            check(Token::Kind::comma);
            auto target = ArgumentQreg();
            check(Token::Kind::semicolon);

            // return corresponding operation
            if (control.second == 1 && target.second == 1) {
                return std::make_unique<qc::StandardOperation>(nqubits, qc::Control(control.first), target.first, qc::X);
            } else {
                auto gate = qc::CompoundOperation(nqubits);
                if (control.second == target.second) {
                    for (unsigned short i = 0; i < target.second; ++i)
                        gate.emplace_back<qc::StandardOperation>(nqubits, qc::Control(control.first + i), target.first+i, qc::X);
                } else if (control.second == 1) {
                    // TODO: multiple targets could be useful here
                    for (unsigned short i = 0; i < target.second; ++i)
                        gate.emplace_back<qc::StandardOperation>(nqubits, qc::Control(control.first), target.first + i, qc::X);
                } else if (target.second == 1) {
                    for (unsigned short i = 0; i < control.second; ++i)
                        gate.emplace_back<qc::StandardOperation>(nqubits, qc::Control(control.first + i), target.first, qc::X);
                } else {
	                error("Register size does not match for CX gate!");
                }
                return std::make_unique<qc::CompoundOperation>(gate);
            }

        } else if (sym == Token::Kind::identifier) {
            scan();
	        auto gateName = t.str;
	        auto cGateName = gateName;
	        unsigned int ncontrols = 0;
	        while (cGateName.front() == 'c') {
		        cGateName = cGateName.substr(1);
		        ncontrols++;
	        }

	        auto gateIt = compoundGates.find(gateName);
	        auto cGateIt = compoundGates.find(cGateName);
            if (gateIt != compoundGates.end() || cGateIt != compoundGates.end()) {
                std::vector<Expr*> parameters;
                std::vector<std::pair<unsigned short , unsigned short>> arguments;
                if (sym == Token::Kind::lpar) {
                    scan();
                    if (sym != Token::Kind::rpar)
                        ExpList(parameters);
                    check(Token::Kind::rpar);
                }
                ArgList(arguments);
                check(Token::Kind::semicolon);

                // return corresponding operation
                registerMap argMap;
                std::map<std::string, Expr*> paramMap;
                unsigned short size = 1;
                if (gateIt != compoundGates.end()) {
	                for (size_t i = 0; i < arguments.size(); ++i) {
		                argMap[gateIt->second.argumentNames[i]] = arguments[i];
		                if (arguments[i].second > 1 && size != 1 && arguments[i].second != size)
			                error("Register sizes do not match!");

		                if (arguments[i].second > 1)
			                size = arguments[i].second;
	                }

	                for (size_t i = 0; i < parameters.size(); ++i)
		                paramMap[gateIt->second.parameterNames[i]] = parameters[i];
                } else { // controlled Gate treatment
                	if (arguments.size() > ncontrols + 1) {
		                error("Too many arguments for controlled gate! Expected " + std::to_string(ncontrols) + "+1, but got " + std::to_string(arguments.size()));
                	}

	                for (size_t i = 0; i < arguments.size(); ++i) {
		                argMap["q"+std::to_string(i)] = arguments[i];
		                if (arguments[i].second > 1 && size != 1 && arguments[i].second != size)
			                error("Register sizes do not match!");

		                if (arguments[i].second > 1)
			                size = arguments[i].second;
	                }

	                for (size_t i = 0; i < parameters.size(); ++i)
		                paramMap[cGateIt->second.parameterNames[i]] = parameters[i];
                }

                // check if single controlled gate
                if (ncontrols > 0 && size == 1) {
                    // TODO: this could be enhanced for the case that any argument is a register
                    if (cGateIt->second.gates.size() == 1) {
                        std::vector<qc::Control> controls{};
                        for (unsigned int j = 0; j < ncontrols; ++j) {
	                        auto arg = (gateIt != compoundGates.end())? gateIt->second.argumentNames[j]: ("q"+std::to_string(j));
	                        controls.emplace_back(argMap[arg].first);
                        }

	                    auto targ = (gateIt != compoundGates.end())? gateIt->second.argumentNames.back(): ("q"+std::to_string(ncontrols));

	                    // special treatment for Toffoli
                        if (cGateName == "x" && ncontrols > 1) {
	                        return std::make_unique<qc::StandardOperation>(nqubits, controls, argMap[targ].first);
                        }

                        auto cGate = cGateIt->second.gates.front();
                        for (size_t j = 0; j < parameters.size(); ++j)
                            paramMap[cGateIt->second.parameterNames[j]] = parameters[j];

                        if (auto cu = dynamic_cast<Ugate *>(cGate)) {
                            std::unique_ptr<Expr> theta(RewriteExpr(cu->theta, paramMap));
                            std::unique_ptr<Expr> phi(RewriteExpr(cu->phi, paramMap));
                            std::unique_ptr<Expr> lambda(RewriteExpr(cu->lambda, paramMap));

                            return std::make_unique<qc::StandardOperation>(nqubits, controls, argMap[targ].first, qc::U3, lambda->num, phi->num, theta->num);
                        } else {
	                        error("Cast to u-Gate not possible for controlled operation.");
                        }
                    }
                } else if (gateIt == compoundGates.end()) {
	                error("Controlled operation for which no definition could be found or which acts on whole qubit register.");
                }

                // identifier specifies just a single operation (U3 or CX)
                if (gateIt->second.gates.size() == 1) {
                    auto gate = gateIt->second.gates.front();
                    if (auto u = dynamic_cast<Ugate *>(gate)) {
                        std::unique_ptr<Expr> theta(RewriteExpr(u->theta, paramMap));
                        std::unique_ptr<Expr> phi(RewriteExpr(u->phi, paramMap));
                        std::unique_ptr<Expr> lambda(RewriteExpr(u->lambda, paramMap));

                        if (argMap[u->target].second == 1) {
                            return std::make_unique<qc::StandardOperation>(nqubits, argMap[u->target].first, qc::U3, lambda->num, phi->num, theta->num);
                        }
                    } else if (auto cx = dynamic_cast<CXgate *>(gate)) {
                        if (argMap[cx->control].second == 1 && argMap[cx->target].second == 1) {
                            return std::make_unique<qc::StandardOperation>(nqubits, qc::Control(argMap[cx->control].first), argMap[cx->target].first, qc::X);
                        }
                    }
                }

                qc::CompoundOperation op(nqubits);
                for (auto& gate: gateIt->second.gates) {
                    if (auto u = dynamic_cast<Ugate*>(gate)) {
                        std::unique_ptr<Expr> theta(RewriteExpr(u->theta, paramMap));
                        std::unique_ptr<Expr> phi(RewriteExpr(u->phi, paramMap));
                        std::unique_ptr<Expr> lambda(RewriteExpr(u->lambda, paramMap));

                        if (argMap[u->target].second == 1) {
                            op.emplace_back<qc::StandardOperation>(nqubits, argMap[u->target].first, qc::U3, lambda->num, phi->num, theta->num);
                        } else {
                            // TODO: multiple targets could be useful here
                            for (unsigned short j = 0; j < argMap[u->target].second; ++j) {
                                op.emplace_back<qc::StandardOperation>(nqubits, argMap[u->target].first + j, qc::U3, lambda->num, phi->num, theta->num);
                            }
                        }
                    } else if (auto cx = dynamic_cast<CXgate*>(gate)) {
                        if (argMap[cx->control].second == 1 && argMap[cx->target].second == 1) {
                            op.emplace_back<qc::StandardOperation>(nqubits, qc::Control(argMap[cx->control].first), argMap[cx->target].first, qc::X);
                        } else if (argMap[cx->control].second == argMap[cx->target].second) {
                            for (unsigned short j = 0; j < argMap[cx->target].second; ++j)
                                op.emplace_back<qc::StandardOperation>(nqubits, qc::Control(argMap[cx->control].first + j), argMap[cx->target].first + j, qc::X);
                        } else if (argMap[cx->control].second == 1) {
                            // TODO: multiple targets could be useful here
                            for (unsigned short k = 0; k < argMap[cx->target].second; ++k)
                                op.emplace_back<qc::StandardOperation>(nqubits,qc::Control(argMap[cx->control].first), argMap[cx->target].first + k, qc::X);
                        } else if (argMap[cx->target].second == 1) {
                            for (unsigned short l = 0; l < argMap[cx->control].second; ++l)
                                op.emplace_back<qc::StandardOperation>(nqubits, qc::Control(argMap[cx->control].first + l), argMap[cx->target].first, qc::X);
                        } else {
	                        error("Register size does not match for CX gate!");
                        }
                    }
                }
                return std::make_unique<qc::CompoundOperation>(op);
            } else {
	            error("Undefined gate " + t.str);
            }
        } else {
	        error("Symbol " + qasm::KindNames[sym] + " not expected in Gate() routine!");
        }
    }

    void Parser::OpaqueGateDecl() {
        check(Token::Kind::opaque);
        check(Token::Kind::identifier);

        CompoundGate gate;
        auto gateName = t.str;
        if (sym == Token::Kind::lpar) {
            scan();
            if (sym != Token::Kind::rpar) {
                IdList(gate.argumentNames);
            }
            check(Token::Kind::rpar);
        }
        IdList(gate.argumentNames);
        compoundGates[gateName] = gate;
        check(Token::Kind::semicolon);
    }

    void Parser::GateDecl() {
        check(Token::Kind::gate);
        check(Token::Kind::identifier);

        CompoundGate gate;
        std::string gateName = t.str;
        if (sym == Token::Kind::lpar) {
            scan();
            if (sym != Token::Kind::rpar) {
                IdList(gate.parameterNames);
            }
            check(Token::Kind::rpar);
        }
        IdList(gate.argumentNames);
        check(Token::Kind::lbrace);

        while (sym != Token::Kind::rbrace) {
            if (sym == Token::Kind::ugate) {
                scan();
                check(Token::Kind::lpar);
                Expr *theta = Exp();
                check(Token::Kind::comma);
                Expr *phi = Exp();
                check(Token::Kind::comma);
                Expr *lambda = Exp();
                check(Token::Kind::rpar);
                check(Token::Kind::identifier);

                gate.gates.push_back(new Ugate(theta, phi, lambda, t.str));
                check(Token::Kind::semicolon);
            } else if (sym == Token::Kind::cxgate) {
                scan();
                check(Token::Kind::identifier);
                std::string control = t.str;
                check(Token::Kind::comma);
                check(Token::Kind::identifier);
                gate.gates.push_back(new CXgate(control, t.str));
                check(Token::Kind::semicolon);

            } else if (sym == Token::Kind::identifier) {
                scan();
                std::string name = t.str;

                std::vector<Expr *> parameters;
                std::vector<std::string> arguments;
                if (sym == Token::Kind::lpar) {
                    scan();
                    if (sym != Token::Kind::rpar) {
                        ExpList(parameters);
                    }
                    check(Token::Kind::rpar);
                }
                IdList(arguments);
                check(Token::Kind::semicolon);

                CompoundGate g = compoundGates[name];
                std::map<std::string, std::string> argsMap;
                for (unsigned long i = 0; i < arguments.size(); i++) {
                    argsMap[g.argumentNames[i]] = arguments[i];
                }

                std::map<std::string, Expr *> paramsMap;
                for (unsigned long i = 0; i < parameters.size(); i++) {
                    paramsMap[g.parameterNames[i]] = parameters[i];
                }

                for (auto & it : g.gates) {
                    if (auto u = dynamic_cast<Ugate *>(it)) {
                        gate.gates.push_back(new Ugate(RewriteExpr(u->theta, paramsMap), RewriteExpr(u->phi, paramsMap), RewriteExpr(u->lambda, paramsMap), argsMap[u->target]));
                    } else if (auto cx = dynamic_cast<CXgate *>(it)) {
                        gate.gates.push_back(new CXgate(argsMap[cx->control], argsMap[cx->target]));
                    } else {
	                    error("Unexpected gate in GateDecl!");
                    }
                }

                for (auto & parameter : parameters) {
                    delete parameter;
                }
            } else if (sym == Token::Kind::barrier) {
                scan();
                std::vector<std::string> arguments;
                IdList(arguments);
                check(Token::Kind::semicolon);
                //Nothing to do here for the simulator
            } else {
	            error("Error in gate declaration!");
            }
        }
        compoundGates[gateName] = gate;
        check(Token::Kind::rbrace);
    }

    std::unique_ptr<qc::Operation> Parser::Qop() {
        if (sym == Token::Kind::ugate || sym == Token::Kind::cxgate || sym == Token::Kind::swap || sym == Token::Kind::identifier)
            return Gate();
        else if (sym == Token::Kind::measure) {
            scan();
            auto qreg = ArgumentQreg();
            check(Token::Kind::minus);
            check(Token::Kind::gt);
            auto creg = ArgumentCreg();
            check(Token::Kind::semicolon);

            if (qreg.second == creg.second) {
                std::vector<unsigned short> qubits, classics;
                for (int i = 0; i < qreg.second; ++i) {
                    qubits.emplace_back(qreg.first + i);
                    classics.emplace_back(creg.first + i);
                }
                return std::make_unique<qc::NonUnitaryOperation>(nqubits, qubits, classics);
            } else {
	            error("Mismatch of qreg and creg size in measurement");
            }
        } else if (sym == Token::Kind::reset) {
            scan();
            auto qreg = ArgumentQreg();
            check(Token::Kind::semicolon);

            std::vector<unsigned short> qubits;
            for (int i = 0; i < qreg.second; ++i) {
                qubits.emplace_back(qreg.first + i);
            }
            return std::make_unique<qc::NonUnitaryOperation>(nqubits, qubits);
        } else {
	        error("No valid Qop: " + t.str);
        }
    }
 }
