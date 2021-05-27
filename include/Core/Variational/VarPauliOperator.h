/*
Copyright (c) 2017-2020 Origin Quantum Computing. All Right Reserved.
Licensed under the Apache License 2.0

Author: LiYe
Created in 2019-01-23

*/

#ifndef VARPAULIOPERATOR_H
#define VARPAULIOPERATOR_H

#include "Core/Variational/var.h"
#include "Core/Variational/utils.h"
#include "Components/Operator/PauliOperator.h"
#include "Core/Variational/complex_var.h"

QPANDA_BEGIN

using VarPauliOperator = PauliOp<complex_var>;

template<>
inline VarPauliOperator::PauliOp(double value)
{
    Variational::var v(value);
    insertData("", complex_var(v, 0));
}

template<>
inline VarPauliOperator VarPauliOperator::dagger() const
{
    auto tmp_data = m_data;
    for (size_t i = 0; i < tmp_data.size(); i++)
    {
        auto &item = tmp_data[i];
        item.second = complex_var(item.second.real(), -1*item.second.imag());
    }

    return VarPauliOperator(tmp_data);
}

template<>
inline std::string VarPauliOperator::toString() const
{
    std::string str = "{";
    for (size_t i = 0; i < m_data.size(); i++)
    {
        str += "\n";

        auto item = m_data[i];
        auto pair = item.first;
        auto value = item.second;

        str += "\"" + pair.second + "\" : [var type]";
    }

    if (!m_data.empty())
    {
        str += "\n";
    }

    str += "}";
    return str;
}

template<>
inline VarPauliOperator::PauliItem VarPauliOperator::genPauliItem(const QTerm &map_i,
                            const QTerm &map_j,
                            const complex_var &value) const
{
    auto tmp_map = map_i;
    auto result = value;
    auto iter_j = map_j.begin();
    for (; iter_j != map_j.end(); iter_j++)
    {
        auto iter_t = tmp_map.find(iter_j->first);
        if (iter_t == tmp_map.end())
        {
            tmp_map.insert(*iter_j);
        }
        else
        {
            auto tmp_value = result;
            std::string tmp = iter_t->second + std::string() +
                iter_j->second;
            if (("XX" == tmp) ||
                ("YY" == tmp) ||
                ("ZZ" == tmp))
            {
                tmp_map.erase(iter_t->first);
            }
            else if ("XY" == tmp)
            {
                result = complex_var(-1*tmp_value.imag(), tmp_value.real());
//                result.first = -1*tmp_value.second;
//                result.second = tmp_value.first;
//                result *= complex_d{0, 1};
                iter_t->second = 'Z';
            }
            else if ("XZ" == tmp)
            {
                result = complex_var(tmp_value.imag(), -1*tmp_value.real());
//                result.first = tmp_value.second;
//                result.second = tmp_value.first*-1;
//                result *= complex_d{0, -1};
                iter_t->second = 'Y';
            }
            else if ("YX" == tmp)
            {
                result = complex_var(tmp_value.imag(), -1*tmp_value.real());
//                result.first = tmp_value.second;
//                result.second = tmp_value.first*-1;
//                result *= complex_d{0, -1};
                iter_t->second = 'Z';
            }
            else if ("YZ" == tmp)
            {
                result = complex_var(-1*tmp_value.imag(), tmp_value.real());
//                result.first = -1*tmp_value.second;
//                result.second = tmp_value.first;
//                result *= complex_d{0, 1};
                iter_t->second = 'X';
            }
            else if ("ZX" == tmp)
            {
                result = complex_var(-1*tmp_value.imag(), tmp_value.real());
//                result.first = -1*tmp_value.second;
//                result.second = tmp_value.first;
//                result *= complex_d{0, 1};
                iter_t->second = 'Y';
            }
            else if ("ZY" == tmp)
            {
                result = complex_var(tmp_value.imag(), -1*tmp_value.real());
//                result.first = tmp_value.second;
//                result.second = tmp_value.first*-1;
//                result *= complex_d{0, -1};
                iter_t->second = 'X';
            }
            else
            {
                std::string err = "Bad para in QPauli.";
                std::cout << err << std::endl;
                throw err;
            }
        }
    }

    QPauliPair pair;
    pair.first = tmp_map;
    pair.second = QTerm2StdString(tmp_map);

    return std::make_pair(pair, result);
}

template<>
inline void VarPauliOperator::reduceDuplicates()
{
    std::map<std::string, complex_var> data_map;
    std::map<std::string, QTerm> term_map;

    for (size_t i = 0; i < m_data.size(); i++)
    {
        auto item = m_data[i];
        auto pair = item.first;
        auto value = item.second;
        QTerm term = pair.first;
        std::string str = pair.second;

        auto result = data_map.find(str);
        if (result != data_map.end())
        {
            result->second = complex_var(result->second.real() + value.real(),
                                         result->second.imag() + value.imag());
//            result->second.first = result->second.first + value.first;
//            result->second.second = result->second.second + value.second;
        }
        else
        {
            data_map.insert(std::make_pair(str, value));
            term_map.insert(std::make_pair(str, term));
        }
    }

    PauliData pauli_data;
    for (auto iter = data_map.begin(); iter != data_map.end(); iter++)
    {
        QPauliPair pair;
        pair.first = term_map[iter->first];
        pair.second = iter->first;

        pauli_data.emplace_back(std::make_pair(pair, iter->second));
    }

    m_data = std::move(pauli_data);
}

template<>
inline QHamiltonian VarPauliOperator::toHamiltonian(bool *ok) const
{
    QHamiltonian hamiltonian;

    for (size_t i = 0; i < m_data.size(); i++)
    {
        auto item = m_data[i];
        auto pair = item.first;
        auto value = item.second;

        auto real = Variational::eval(value.real(), true)(0, 0);
        auto imag = Variational::eval(value.imag(), true)(0, 0);

        if (fabs(imag) > fabs(m_error_threshold))
        {
            std::cout << "PauliOperator data cannot convert to Hamiltonian."
                      << std::endl;
            if (ok)
            {
                *ok = false;
            }

            return QHamiltonian();
        }

        if (fabs(real) < fabs(m_error_threshold))
        {
            continue;
        }

        hamiltonian.emplace_back(std::make_pair(pair.first, real));
    }

    if (ok)
    {
        *ok = true;
    }

    return hamiltonian;
}

QPANDA_END
#endif // VARPAULIOPERATOR_H
