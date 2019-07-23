/**
 * @file    qx_simulator.h
 * @author	Nader Khammassi
 *          Imran Ashraf
 * @date	   23-12-16
 * @brief	qx simulator interface
 */

#ifndef QX_SIMULATOR_H
#define QX_SIMULATOR_H

#include <xpu.h>
#include <xpu/runtime>

#include <core/circuit.h>
#include <qx_representation.h>
#include <libqasm_interface.h>
#include <qasm_semantic.hpp>
#include <qx_version.h>

#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdlib>
#include <map>
#include <stdint.h>


/**
 * i/o helpers
 */

#define println(x) std::cout <<"[QXELERATOR]" << __FILE__ << ":" << __LINE__ << " " << x << std::endl
#define error(x) std::cerr <<"[QXELERATOR]" << __FILE__ << ":" << __LINE__ << " Error:" << x << std::endl

namespace qx
{

/**
 * simulator
 */
class simulator
{
protected:
    qx::qu_register * reg;

private:
    size_t                     qubits = 0;
    std::vector<qx::circuit*>  perfect_circuits;
    std::vector<qx::circuit*>  noisy_circuits;
    qx::error_model_t          error_model       = qx::__unknown_error_model__;
    size_t                     total_errors      = 0;
    double                     error_probability = 0;

public:
    simulator() : reg(nullptr) 
    {
        xpu::init(); 
    }

    ~simulator()
    { 
        for ( auto vec : {perfect_circuits, noisy_circuits} )
        {
            for (auto ptr : vec)
                delete ptr;
        }
        perfect_circuits.clear();
        noisy_circuits.clear();

        xpu::clean();
    }

    void set(std::string file_path)
    {
        FILE * qasm_file = fopen(file_path.c_str(), "r");
        if (!qasm_file)
        {
            error("Could not open " << file_path );
            return;
        }


        // construct libqasm parser and safely parse input file
        compiler::QasmRepresentation ast;
        compiler::QasmSemanticChecker * parser;
        try
        {
            parser = new compiler::QasmSemanticChecker(qasm_file);
            ast = parser->getQasmRepresentation();
        }
        catch (std::exception &e)
        {
            error("parsing file " << file_path);
            error(e.what());
        }
        delete parser;
        fclose(qasm_file);

        // set number of qubits
        qubits = ast.numQubits();

        // check whether an error model is specified
        if (ast.getErrorModelType() == "depolarizing_channel")
        {
            error_probability = ast.getErrorModelParameters().at(0);
            error_model       = qx::__depolarizing_channel__;
        }

        for ( auto vec : {perfect_circuits, noisy_circuits} )
        {
            for (auto ptr : vec)
                delete ptr;
        }
        perfect_circuits.clear();
        noisy_circuits.clear();

        // convert libqasm ast to qx internal representation
        std::vector<compiler::SubCircuit> subcircuits = ast.getSubCircuits().getAllSubCircuits();
        for(auto subcircuit : subcircuits)
        {
            // libqasm is detecting default subcircuit for some reason
            // TODO: report it on libqasm github
            if( subcircuit.nameSubCircuit() == "default")
                continue;

            try
            {
                perfect_circuits.push_back(load_cqasm_code(qubits, subcircuit));
            }
            catch (std::string type)
            {
                std::cerr << "Encountered unsupported gate: " << type << std::endl;
                xpu::clean();
            }
        }

        if (error_model == qx::__depolarizing_channel__)
        {
            // println("[+] generating noisy circuits (p=" << qxr.getErrorProbability() << ")...");
            for (size_t i=0; i<perfect_circuits.size(); i++)
            {
                if (perfect_circuits[i]->size() == 0)
                    continue;

                size_t iterations = perfect_circuits[i]->get_iterations();
                for (size_t it=0; it<iterations; ++it)
                    noisy_circuits.push_back(qx::noisy_dep_ch(perfect_circuits[i],error_probability,total_errors));
            }
            // println("[+] total errors injected in all circuits : " << total_errors);
        }
    }


    /**
     * execute qasm file
     */
    void execute(size_t navg)
    {
        // create the quantum state
        // println("Creating quantum register of " << qubits << " qubits... ");
        try
        {
            reg = new qx::qu_register(qubits);
        }
        catch(std::bad_alloc& exception)
        {
            std::cerr << "Not enough memory, aborting" << std::endl;
            xpu::clean();
        }
        catch(std::exception& exception)
        {
            std::cerr << "Unexpected exception (" << exception.what() << "), aborting" << std::endl;
            xpu::clean();
        }


        // measurement averaging
        if (navg)
        {
            if (error_model == qx::__depolarizing_channel__)
            {
                qx::measure m;
                for (size_t s=0; s<navg; ++s)
                {
                    reg->reset();
                    for (size_t i=0; i<noisy_circuits.size(); i++)
                    {
                        if (noisy_circuits[i]->size() == 0)
                            continue;

                        noisy_circuits[i]->execute(*reg,false,true);
                    }
                    m.apply(*reg);
                }
            }
            else
            {
                qx::measure m;
                for (size_t s=0; s<navg; ++s)
                {
                    reg->reset();
                    for (size_t i=0; i<perfect_circuits.size(); i++)
                        perfect_circuits[i]->execute(*reg,false,true);
                    m.apply(*reg);
                }
            }
            // println("Average measurement after " << navg << " shots:");
            // reg->dump(true);
        }
        else
        {
            if (error_model == qx::__depolarizing_channel__)
            {
                // println("[+] generating noisy circuits (p=" << qxr.getErrorProbability() << ")...");
                for (size_t i=0; i<noisy_circuits.size(); i++)
                {
                    if (noisy_circuits[i]->size() == 0)
                        continue;

                    noisy_circuits[i]->execute(*reg);
                }
            }
            else
            {
                for (size_t i=0; i<perfect_circuits.size(); i++)
                {
                    perfect_circuits[i]->execute(*reg);
                }
            }
        }
    }

    bool move(size_t q)
    {
        return reg->get_measurement(q);
    }

    std::string get_state()
    {
        return reg->get_state();
    }
};
}

#endif // QX_SIMULATOR_H
