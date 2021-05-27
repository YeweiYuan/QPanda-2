/**
 * @file var.h
 * @author Agony5757 (Agony5757@github.com)
 * @brief Variational quantum-classical hybrid operations.
 * @date 2018-12-18
 *
 * @copyright Copyright Origin Quantum(c) 2018
 *
 */
#ifndef VAR_H
#define VAR_H

#include <iostream>
#include <vector>
#include <memory>
#include <utility>
#include <Eigen/Dense>
#include <unordered_set>
#include <unordered_map>
#include <map>
#include <type_traits>
#include "Core/QuantumCircuit/ClassicalProgram.h"
#include "Core/Utilities/QPandaNamespace.h"
#include "Core/QuantumCircuit/QProgram.h"
#include "Core/QuantumCircuit/QCircuit.h"
#include "Core/QuantumCircuit/QGate.h"
#include "Core/QuantumMachine/OriginQuantumMachine.h"
#include "Components/Operator/PauliOperator.h"

using Eigen::MatrixXd;
using Eigen::VectorXd;
using Eigen::ArrayXd;

QPANDA_BEGIN
namespace Variational {

    class var;

    /**
     * @brief enum class of operator types
     *
     */
    enum class op_type : int {
        plus,
        minus,
        multiply,
        divide,
        exponent,
        log,
        polynomial,
        dot,
        inverse,
        transpose,
        sum,
        stack,
        subscript,
        qop,
        qop_pmeasure,
        qop_real_chip,
        qop_pmeasure_real_chip,
        sigmoid,
        softmax,
        cross_entropy,
        dropout,
        none
    };

    int numOpArgs(op_type op);
}
QPANDA_END

namespace std {

    /**
     * @brief hash function to enable unordered_map<var, _Ty>
     * and unordered_set<var>
     */
    template <> struct hash<QPanda::Variational::var> {
        size_t operator()(const QPanda::Variational::var&) const;
    };
}


QPANDA_BEGIN
namespace Variational {

    /**
     * @brief implementation class for the var. Impl only includes
     * classical operator with fixed number of arguments.
     *
     */
    struct impl {
    public:
        /**
         * @brief Construct from a Eigen matrix
         *
         */
        impl(const MatrixXd&);
        impl(const MatrixXd&, bool isDifferentiable);

        /**
         * @brief Construct from a operator
         *
         */
        impl(op_type, const std::vector<var>&);

        /**
         * @brief Internal value
         *
         */
        MatrixXd val;

        /**
         * @brief Placeholder/Variable
         *
         */
        bool m_is_differentiable;

        /**
         * @brief Operator type
         *
         */
        op_type op;

        /**
         * @brief Childrens. For example, c = a + b.
         * c is a and b's parent, a and b are c's children
         *
         */
        std::vector<var> children;

        /**
         * @brief Parents. For example, c = a + b.
         * c is a and b's parent, a and b are c's children
         *
         */
        std::vector<std::weak_ptr<impl>> parents;

        /**
        * @brief Internal value
        *
        */
        MatrixXd m_prob;

        /**
         * @brief Destroy the impl object
         *
         */
        virtual ~impl() = default;
    };
    /**
     * @brief Implementation class for the stack operation.
     *
     */
    struct impl_stack : public impl {
    public:

        /**
         * @brief Construct a new impl stack object by the axis
         * and children. y = stack(axis=0, [a,b,c,d]). It will
         * try to place a,b,c,d into one matrix with the same columns,
         * if axis==1, the same rows.
         *
         * @param axis the stack axis.
         */
        impl_stack(int axis, const std::vector<var>&);

        /**
         * @brief stack axis, should be either 0 or 1.
         *
         */
        int m_axis;
    };

    /**
     * @brief implementation for the subscript operation.
     */
    struct impl_subscript : public impl {
    public:

        /**
         * @brief Construct a new impl subscript object by child
         * and the subscript. c = a[i], subscript=i, a=children
         * and c=parent
         *
         * @param subscript the subscript.
         */
        impl_subscript(int subscript, const std::vector<var>&);

        /**
         * @brief the subscript
         *
         */
        int m_subscript;
    };

    /**
     * @brief The class denotes the variable
     *
     */
    class var {
    public:

        /**
         * @brief Construct a new var object by the impl object
         *
         */
        var(std::shared_ptr<impl>);
        /**
         * @brief Construct a new var object by a double.
         *
         */
        var(double);

        /**
         * @brief Construct a new var object by a Eigen matrix
         *
         */
        var(const MatrixXd&);

        var(double, bool);
        var(const MatrixXd&, bool);

        /**
         * @brief Construct a new var object by the operator type
         * and children
         *
         * @param op operator type
         * @param children children of the operator. For example,
         * c = a + b. c is a and b's parent, a and b are c's children
         */
        var(op_type op, const std::vector<var>& children);

        /**
         * @brief move constructor of var
         *
         */
        var(var&&);

        /**
         * @brief
         *
         * @return var&
         */
        var& operator=(var&&);
        var(const var&);
        var& operator=(const var&);
        var clone();
        virtual size_t getNumOpArgs();
        MatrixXd getValue() const;
        void setValue(const MatrixXd&);
        op_type getOp() const;
        void setOp(op_type);
        std::vector<var>& getChildren() const;
        std::vector<var> getParents() const;
        long getUseCount() const;
        bool getValueType() const;
        MatrixXd _eval();
        MatrixXd _back_single(const MatrixXd& dx, size_t op_idx);
        std::vector<MatrixXd> _back(const MatrixXd& dx, const std::unordered_set<var>& nonconsts);
        std::vector<MatrixXd> _back(const MatrixXd& dx);
        bool operator==(const var& rhs) const;
        friend struct std::hash<var>;
        template <typename... V>
        friend const var pack_expression(op_type, V&...);
        template <typename... V>
        friend const var pack_expression(op_type op, int axis, V&...);
        inline const var operator[](int subscript) {
            std::vector<std::shared_ptr<impl> > vimpl = { this->pimpl };
            std::vector<var> v;
            for (const std::shared_ptr<impl>& _impl : vimpl) {
                v.emplace_back(_impl);
            }
            var res(std::make_shared<impl_subscript>(subscript, v));
            for (const std::shared_ptr<impl>& _impl : vimpl) {
                _impl->parents.push_back(res.pimpl);
            }
            return res;
        }
        friend const var py_stack(int axis, std::vector<var>& args);
        std::shared_ptr<impl> pimpl;

        ~var();
    };

    class VariationalQuantumGate
    {
    public:
    protected:
        std::vector<var> m_vars;
        std::vector<double> m_constants;
        bool m_is_dagger;
        QVec m_control_qubit;
    public:
        /**
         * @brief
         *
         * @return size_t the number of vars.
         */
        inline size_t n_var() { return m_vars.size(); }

        /**
         * @brief Get all variables for the VQG.
         *
         * @return std::vector<Variable>
         */
        const std::vector<var>& get_vars() { return m_vars; }

        const std::vector<double>& get_constants() { return m_constants; }

        /**
         * @brief Get the position for var in the m_vars.
         * If not existed, return -1. Otherwise, return the
         * position n, which is var == m_vars[n].
         *
         * @param var The corresponding variable.
         * @return int -1 if not existed, or position.
         */
        inline int var_pos(var _var) {
            for (size_t i = 0u; i < m_vars.size(); ++i)
            {
                if (m_vars[i] == _var)
                    return (int)i;
            }
            return -1;
        }

        /**
         * @brief Copy Constructor for a new Variational
         *  Quantum Gate object
         *
         */
        VariationalQuantumGate(const VariationalQuantumGate&);

        /**
         * @brief Default Constructor for a new Variational
         *  Quantum Gate object
         *
         */
        VariationalQuantumGate()
        {
            m_is_dagger = false;
        }

        /**
         * @brief Interface to instantialize the QGate with
         * VQG
         *
         * @return QGate Instantiation
         */
        virtual QGate feed() = 0;

        /**
         * @brief Interface to instantialize the QGate with
         * the "offset".
         *
         * @param offset <number of variable, offset>
         *
         * @return QGate
         */
        virtual QGate feed(std::map<size_t, double> offset)
        {
            return this->feed();
        }

        /**
         * @brief Destroy the Variational Quantum Gate object
         *
         */
        virtual ~VariationalQuantumGate() {}

        /**
         * @brief Interface to copy the instance, and return a
         * shared_ptr for the object.
         *
         * @return std::shared_ptr<VariationalQuantumGate>
         */
        virtual std::shared_ptr<VariationalQuantumGate> copy() = 0;

        virtual bool set_dagger(bool dagger)
        {
            m_is_dagger = dagger;
            return dagger;
        }

        virtual bool set_control(QVec control_qubit)
        {
            m_control_qubit.assign(control_qubit.begin(), control_qubit.end());
            return true;
        }

        virtual bool is_dagger()
        {
            return m_is_dagger;
        }

        virtual QVec get_control_qubit()
        {
            QVec qv;
            qv.assign(m_control_qubit.begin(), m_control_qubit.end());
            return qv;
        }

        virtual void copy_dagger_and_control_qubit(QGate & gate)
        {
            gate.setDagger(m_is_dagger);
            gate.setControl(m_control_qubit);
        }
        virtual void copy_dagger_and_control_qubit(std::shared_ptr<VariationalQuantumGate> gate)
        {
            gate->set_dagger(m_is_dagger);
            gate->set_control(m_control_qubit);
        }

    };

    class VariationalQuantumGate_H : public VariationalQuantumGate
    {
    private:
        Qubit* m_q;
    public:
        explicit VariationalQuantumGate_H(Qubit* q);
        VariationalQuantumGate_H(Qubit* q, bool is_dagger);
        VariationalQuantumGate_H(Qubit* q, bool is_dagger, QVec control_qubit);
        VariationalQuantumGate_H(const VariationalQuantumGate_H & gate) :
            m_q(gate.m_q)
        {
            m_vars = gate.m_vars;
            m_constants = gate.m_constants;
            m_control_qubit = gate.m_control_qubit;
            m_is_dagger = gate.m_is_dagger;
        }


        inline QGate feed()
        {
            QGate h = H(m_q);
            copy_dagger_and_control_qubit(h);
            return h;
        }
        inline std::shared_ptr<VariationalQuantumGate> copy()
        {
            auto shared_ptr_h = std::make_shared<VariationalQuantumGate_H>(m_q);
            copy_dagger_and_control_qubit(shared_ptr_h);
            return shared_ptr_h;
        }
        inline VariationalQuantumGate_H dagger()
        {
            auto temp = VariationalQuantumGate_H(*this);
            temp.m_is_dagger = temp.m_is_dagger ^ true;
            return temp;
        }

        inline VariationalQuantumGate_H control(QVec qv)
        {
            auto temp = VariationalQuantumGate_H(*this);
            temp.m_control_qubit.assign(qv.begin(), qv.end());
            return temp;
        }
    };

    class VariationalQuantumGate_X : public VariationalQuantumGate
    {
    private:
        Qubit* m_q;
    public:
        explicit VariationalQuantumGate_X(Qubit* q);
        VariationalQuantumGate_X(const VariationalQuantumGate_X & gate) :
            m_q(gate.m_q)
        {
            m_vars = gate.m_vars;
            m_constants = gate.m_constants;
            m_control_qubit = gate.m_control_qubit;
            m_is_dagger = gate.m_is_dagger;
        }
        inline QGate feed()
        {
            QGate x = X(m_q);
            copy_dagger_and_control_qubit(x);
            return x;
        }
        inline std::shared_ptr<VariationalQuantumGate> copy()
        {
            auto shared_ptr_x = std::make_shared<VariationalQuantumGate_X>(m_q);
            copy_dagger_and_control_qubit(shared_ptr_x);
            return shared_ptr_x;
        }

        inline VariationalQuantumGate_X dagger()
        {
            auto temp = VariationalQuantumGate_X(*this);
            temp.m_is_dagger = temp.m_is_dagger ^ true;
            return temp;
        }

        inline VariationalQuantumGate_X control(QVec qv)
        {
            auto temp = VariationalQuantumGate_X(*this);
            temp.m_control_qubit.assign(qv.begin(), qv.end());
            return temp;
        }
    };

    class VariationalQuantumGate_RX : public VariationalQuantumGate
    {
    private:
        Qubit* m_q;
    public:
        explicit VariationalQuantumGate_RX(Qubit*, var);
        explicit VariationalQuantumGate_RX(Qubit*, double angle);
        VariationalQuantumGate_RX(const VariationalQuantumGate_RX & gate) :
            m_q(gate.m_q)
        {
            m_vars = gate.m_vars;
            m_constants = gate.m_constants;
            m_control_qubit = gate.m_control_qubit;
            m_is_dagger = gate.m_is_dagger;
        }
        QGate feed();
        QGate feed(std::map<size_t, double>);
        inline std::shared_ptr<VariationalQuantumGate> copy()
        {
            if (m_vars.size() == 0)
            {
                auto shared_ptr_rx = std::make_shared<VariationalQuantumGate_RX>(m_q, m_constants[0]);
                copy_dagger_and_control_qubit(shared_ptr_rx);
                return shared_ptr_rx;
            }
            else
            {
                auto shared_ptr_rx = std::make_shared<VariationalQuantumGate_RX>(m_q, m_vars[0]);
                copy_dagger_and_control_qubit(shared_ptr_rx);
                return shared_ptr_rx;
            }
        }

        inline VariationalQuantumGate_RX dagger()
        {
            auto temp = VariationalQuantumGate_RX(*this);
            temp.m_is_dagger = temp.m_is_dagger ^ true;
            return temp;
        }

        inline VariationalQuantumGate_RX control(QVec qv)
        {
            auto temp = VariationalQuantumGate_RX(*this);
            temp.m_control_qubit.assign(qv.begin(), qv.end());
            return temp;
        }
    };

    class VariationalQuantumGate_RY : public VariationalQuantumGate
    {
    private:
        Qubit* m_q;
    public:
        explicit VariationalQuantumGate_RY(Qubit*, var);
        explicit VariationalQuantumGate_RY(Qubit*, double angle);
        VariationalQuantumGate_RY(const VariationalQuantumGate_RY & gate) :
            m_q(gate.m_q)
        {
            m_vars = gate.m_vars;
            m_constants = gate.m_constants;
            m_control_qubit = gate.m_control_qubit;
            m_is_dagger = gate.m_is_dagger;
        }
        QGate feed();
        QGate feed(std::map<size_t, double>);
        inline std::shared_ptr<VariationalQuantumGate> copy()
        {
            if (m_vars.size() == 0)
            {
                auto shared_ptr_ry = std::make_shared<VariationalQuantumGate_RY>(m_q, m_constants[0]);
                copy_dagger_and_control_qubit(shared_ptr_ry);
                return shared_ptr_ry;
            }
            else
            {
                auto shared_ptr_ry = std::make_shared<VariationalQuantumGate_RY>(m_q, m_vars[0]);
                copy_dagger_and_control_qubit(shared_ptr_ry);
                return shared_ptr_ry;
            }
        }

        inline VariationalQuantumGate_RY dagger()
        {
            auto temp = VariationalQuantumGate_RY(*this);
            temp.m_is_dagger = temp.m_is_dagger ^ true;
            return temp;
        }

        inline VariationalQuantumGate_RY control(QVec qv)
        {
            auto temp = VariationalQuantumGate_RY(*this);
            temp.m_control_qubit.assign(qv.begin(), qv.end());
            return temp;
        }
    };

    class VariationalQuantumGate_RZ : public VariationalQuantumGate
    {
    private:
        Qubit* m_q;
    public:
        explicit VariationalQuantumGate_RZ(Qubit*, var);
        explicit VariationalQuantumGate_RZ(Qubit*, double angle);
        VariationalQuantumGate_RZ(const VariationalQuantumGate_RZ & gate) :
            m_q(gate.m_q)
        {
            m_vars = gate.m_vars;
            m_constants = gate.m_constants;
            m_control_qubit = gate.m_control_qubit;
            m_is_dagger = gate.m_is_dagger;
        }
        QGate feed();
        QGate feed(std::map<size_t, double>);
        inline std::shared_ptr<VariationalQuantumGate> copy()
        {
            if (m_vars.size() == 0)
            {
                auto shared_ptr_rz = std::make_shared<VariationalQuantumGate_RZ>(m_q, m_constants[0]);
                copy_dagger_and_control_qubit(shared_ptr_rz);
                return shared_ptr_rz;
            }
            else
            {
                auto shared_ptr_rz = std::make_shared<VariationalQuantumGate_RZ>(m_q, m_vars[0]);
                copy_dagger_and_control_qubit(shared_ptr_rz);
                return shared_ptr_rz;
            }
        }

        inline VariationalQuantumGate_RZ dagger()
        {
            auto temp = VariationalQuantumGate_RZ(*this);
            temp.m_is_dagger = temp.m_is_dagger ^ true;
            return temp;
        }

        inline VariationalQuantumGate_RZ control(QVec qv)
        {
            auto temp = VariationalQuantumGate_RZ(*this);
            temp.m_control_qubit.assign(qv.begin(), qv.end());
            return temp;
        }
    };
    class VariationalQuantumGate_CRX : public VariationalQuantumGate
    {
    private:
        Qubit* m_target;

    public:
        explicit VariationalQuantumGate_CRX(Qubit*, QVec, var);
        explicit VariationalQuantumGate_CRX(Qubit*, QVec, double angle);
        VariationalQuantumGate_CRX(const VariationalQuantumGate_CRX & old);
        inline QGate feed();
        QGate feed(std::map<size_t, double>);
        inline std::shared_ptr<VariationalQuantumGate> copy()
        {
            if (m_vars.size() == 0)
            {
                auto shared_ptr_crx = std::make_shared<VariationalQuantumGate_CRX>(m_target, m_control_qubit, m_constants[0]);
                copy_dagger_and_control_qubit(shared_ptr_crx);
                return shared_ptr_crx;
            }
            else
            {
                auto shared_ptr_crx = std::make_shared<VariationalQuantumGate_CRX>(m_target, m_control_qubit, m_vars[0]);
                copy_dagger_and_control_qubit(shared_ptr_crx);
                return shared_ptr_crx;
            }

        }

        inline VariationalQuantumGate_CRX dagger()
        {
            auto temp = VariationalQuantumGate_CRX(*this);
            temp.m_is_dagger = temp.m_is_dagger ^ true;
            return temp;
        }

        inline VariationalQuantumGate_CRX control(QVec qv)
        {
            auto temp = VariationalQuantumGate_CRX(*this);
            temp.m_control_qubit.assign(qv.begin(), qv.end());
            return temp;
        }
    };
    class VariationalQuantumGate_CRY : public VariationalQuantumGate
    {
    private:
        Qubit* m_target;
    public:
        explicit VariationalQuantumGate_CRY(Qubit*, QVec, var);
        explicit VariationalQuantumGate_CRY(Qubit*, QVec, double angle);
        VariationalQuantumGate_CRY(const VariationalQuantumGate_CRY & old);
        inline QGate feed();
        QGate feed(std::map<size_t, double>);
        inline std::shared_ptr<VariationalQuantumGate> copy()
        {
            if (m_vars.size() == 0)
            {
                auto shared_ptr_cry = std::make_shared<VariationalQuantumGate_CRY>(m_target, m_control_qubit, m_constants[0]);
                copy_dagger_and_control_qubit(shared_ptr_cry);
                return shared_ptr_cry;
            }
            else
            {
                auto shared_ptr_cry = std::make_shared<VariationalQuantumGate_CRY>(m_target, m_control_qubit, m_vars[0]);
                copy_dagger_and_control_qubit(shared_ptr_cry);
                return shared_ptr_cry;
            }
        }

        inline VariationalQuantumGate_CRY dagger()
        {
            auto temp = VariationalQuantumGate_CRY(*this);
            temp.m_is_dagger = temp.m_is_dagger ^ true;
            return temp;
        }

        inline VariationalQuantumGate_CRY control(QVec qv)
        {
            auto temp = VariationalQuantumGate_CRY(*this);
            temp.m_control_qubit.assign(qv.begin(), qv.end());
            return temp;
        }
    };
    class VariationalQuantumGate_CRZ : public VariationalQuantumGate
    {
    private:
        Qubit* m_target;

    public:
        explicit VariationalQuantumGate_CRZ(Qubit*, QVec, var);
        explicit VariationalQuantumGate_CRZ(Qubit*, QVec, double angle);
        VariationalQuantumGate_CRZ(const VariationalQuantumGate_CRZ & old);
        inline QGate feed();
        QGate feed(std::map<size_t, double>);
        inline std::shared_ptr<VariationalQuantumGate> copy()
        {
            if (m_vars.size() == 0)
            {
                auto shared_ptr_crz = std::make_shared<VariationalQuantumGate_CRZ>(m_target, m_control_qubit, m_constants[0]);
                copy_dagger_and_control_qubit(shared_ptr_crz);
                return shared_ptr_crz;
            }
            else
            {
                auto shared_ptr_crz = std::make_shared<VariationalQuantumGate_CRZ>(m_target, m_control_qubit, m_vars[0]);
                copy_dagger_and_control_qubit(shared_ptr_crz);
                return shared_ptr_crz;
            }
        }

        inline VariationalQuantumGate_CRZ dagger()
        {
            auto temp = VariationalQuantumGate_CRZ(*this);
            temp.m_is_dagger = temp.m_is_dagger ^ true;
            return temp;
        }

        inline VariationalQuantumGate_CRZ control(QVec qv)
        {
            auto temp = VariationalQuantumGate_CRZ(*this);
            temp.m_control_qubit.assign(qv.begin(), qv.end());
            return temp;
        }
    };
    class VariationalQuantumGate_CZ : public VariationalQuantumGate
    {
    private:
        Qubit* m_q1;
        Qubit* m_q2;
    public:
        explicit VariationalQuantumGate_CZ(Qubit*, Qubit*);
        VariationalQuantumGate_CZ(const VariationalQuantumGate_CZ & gate) :
            m_q1(gate.m_q1), m_q2(gate.m_q2)
        {
            m_vars = gate.m_vars;
            m_constants = gate.m_constants;
            m_control_qubit = gate.m_control_qubit;
            m_is_dagger = gate.m_is_dagger;
        }
        inline QGate feed()
        {
            auto cz = CZ(m_q1, m_q2);
            copy_dagger_and_control_qubit(cz);
            return cz;
        }
        inline std::shared_ptr<VariationalQuantumGate> copy()
        {
            auto shared_ptr_cz = std::make_shared<VariationalQuantumGate_CZ>(m_q1, m_q2);
            copy_dagger_and_control_qubit(shared_ptr_cz);
            return shared_ptr_cz;
        }

        inline VariationalQuantumGate_CZ dagger()
        {
            auto temp = VariationalQuantumGate_CZ(*this);
            temp.m_is_dagger = temp.m_is_dagger ^ true;
            return temp;
        }

        inline VariationalQuantumGate_CZ control(QVec qv)
        {
            auto temp = VariationalQuantumGate_CZ(*this);
            temp.m_control_qubit.assign(qv.begin(), qv.end());
            return temp;
        }
    };

    class VariationalQuantumGate_CNOT : public VariationalQuantumGate
    {
    private:
        Qubit* m_q1;
        Qubit* m_q2;
    public:
        explicit VariationalQuantumGate_CNOT(Qubit*, Qubit*);
        VariationalQuantumGate_CNOT(const VariationalQuantumGate_CNOT & gate) :
            m_q1(gate.m_q1), m_q2(gate.m_q2)
        {
            m_vars = gate.m_vars;
            m_constants = gate.m_constants;
            m_control_qubit = gate.m_control_qubit;
            m_is_dagger = gate.m_is_dagger;
        }
        inline QGate feed()
        {
            auto cnot = CNOT(m_q1, m_q2);
            copy_dagger_and_control_qubit(cnot);
            return cnot;
        }
        inline std::shared_ptr<VariationalQuantumGate> copy()
        {
            auto shared_ptr_cnot = std::make_shared<VariationalQuantumGate_CNOT>(m_q1, m_q2);
            copy_dagger_and_control_qubit(shared_ptr_cnot);
            return shared_ptr_cnot;
        }

        inline VariationalQuantumGate_CNOT dagger()
        {
            auto temp = VariationalQuantumGate_CNOT(*this);
            temp.m_is_dagger = temp.m_is_dagger ^ true;
            return temp;
        }

        inline VariationalQuantumGate_CNOT control(QVec qv)
        {
            auto temp = VariationalQuantumGate_CNOT(*this);
            temp.m_control_qubit.assign(qv.begin(), qv.end());
            return temp;
        }
    };

    class VariationalQuantumCircuit
    {
        std::vector<var> m_vars;
        std::vector<std::shared_ptr<VariationalQuantumGate>> m_gates;
        std::unordered_map<var,
            std::vector<std::weak_ptr<VariationalQuantumGate>>> m_var_in_which_gate;
        bool m_is_dagger;
        QVec m_control_qubit;

        void _insert_copied_gate(std::shared_ptr<VariationalQuantumGate> gate);
        void _insert_copied_gate(VariationalQuantumGate &gate);
    public:

        VariationalQuantumCircuit();
        VariationalQuantumCircuit(const VariationalQuantumCircuit&);
        VariationalQuantumCircuit(QCircuit);

        inline std::vector<var> &get_vars()
        {
            return m_vars;
        }

        QCircuit feed(const std::vector<
            std::tuple<std::weak_ptr<VariationalQuantumGate>, size_t, double>>) const;

        QCircuit feed();

        std::vector<std::weak_ptr<VariationalQuantumGate>> get_var_in_which_gate(const var&) const;

        template<typename VQG_Ty>
        VariationalQuantumCircuit& insert(VQG_Ty gate);
        inline bool set_dagger(bool dagger)
        {
            m_is_dagger = dagger;
            return m_is_dagger;
        }

        inline bool set_control(QVec control_qubit)
        {
            m_control_qubit.assign(control_qubit.begin(), control_qubit.end());
            return true;
        }

        inline bool is_dagger()
        {
            return m_is_dagger;
        }

        inline QVec get_control_qubit()
        {
            QVec qv;
            qv.assign(m_control_qubit.begin(), m_control_qubit.end());
            return qv;
        }

        inline VariationalQuantumCircuit dagger()
        {
            auto temp = VariationalQuantumCircuit(*this);
            temp.m_is_dagger = temp.m_is_dagger ^ true;
            return temp;
        }

        inline VariationalQuantumCircuit control(QVec qv)
        {
            auto temp = VariationalQuantumCircuit(*this);
            temp.m_control_qubit.assign(qv.begin(), qv.end());
            return temp;
        }
    private:
        std::shared_ptr<VariationalQuantumGate> qg2vqg(AbstractQGateNode* gate) const;
        VariationalQuantumCircuit qc2vqc(AbstractQuantumCircuit* q) const;

    };

    template<typename VQG_Ty>
    VariationalQuantumCircuit& VariationalQuantumCircuit::insert(VQG_Ty  gate)
    {
        static_assert(std::is_base_of<VariationalQuantumGate, VQG_Ty>::value, "Bad VQG Type");
        auto copy_gate = gate.copy();
        _insert_copied_gate(copy_gate);
        return *this;
    }

    template<>
    VariationalQuantumCircuit& VariationalQuantumCircuit::insert<std::shared_ptr<VariationalQuantumGate>>
        (std::shared_ptr<VariationalQuantumGate> gate);

    template<>
    VariationalQuantumCircuit& VariationalQuantumCircuit::insert<VariationalQuantumCircuit>
        (VariationalQuantumCircuit circuit);

    template<>
    VariationalQuantumCircuit& VariationalQuantumCircuit::insert<QGate&>(QGate &gate);

    template<>
    VariationalQuantumCircuit& VariationalQuantumCircuit::insert<QGate>(QGate gate);

    template<>
    VariationalQuantumCircuit& VariationalQuantumCircuit::insert<QCircuit>(QCircuit c);

    typedef VariationalQuantumGate_H VQG_H;
    typedef VariationalQuantumGate_X VQG_X;
    typedef VariationalQuantumGate_RX VQG_RX;
    typedef VariationalQuantumGate_RY VQG_RY;
    typedef VariationalQuantumGate_RZ VQG_RZ;
    typedef VariationalQuantumGate_CNOT VQG_CNOT;
    typedef VariationalQuantumGate_CZ VQG_CZ;
    typedef VariationalQuantumGate_CRX VQG_CRX;
    typedef VariationalQuantumGate_CRY VQG_CRY;
    typedef VariationalQuantumGate_CRZ VQG_CRZ;


    typedef VariationalQuantumGate VQG;
    typedef VariationalQuantumCircuit VQC;

    struct impl_vqp : public impl {
    public:
        impl_vqp(VariationalQuantumCircuit,
            PauliOperator,
            QuantumMachine*,
            std::vector<Qubit*>);

        impl_vqp(VariationalQuantumCircuit,
            PauliOperator,
            QuantumMachine*,
            std::map<size_t, Qubit*>);

        double _get_gradient(var _var);
        double _get_gradient_one_term(var _var, QTerm);
        double _get_expectation_one_term(QCircuit, QTerm);
        double _get_expectation();

    private:
        std::map<size_t, Qubit*> m_measure_qubits;
        PauliOperator m_op;
        QuantumMachine* m_machine;
        VariationalQuantumCircuit m_circuit;
    };

    struct impl_vqp_real_chip : public impl {
    public:
        impl_vqp_real_chip(VariationalQuantumCircuit,
            PauliOperator,
            QuantumMachine*,
            std::vector<Qubit*>,
            int shots);

        impl_vqp_real_chip(VariationalQuantumCircuit,
            PauliOperator,
            QuantumMachine*,
            std::map<size_t, Qubit*>,
            int shots);

        double _get_gradient(var _var);
        double _get_gradient_one_term(var _var, QTerm);
        double _get_expectation_one_term(QCircuit, QTerm);
        double _get_expectation();

    private:
        int m_shots;
        std::map<size_t, Qubit*> m_measure_qubits;
        PauliOperator m_op;
        QuantumMachine* m_machine;
        VariationalQuantumCircuit m_circuit;
    };

    struct impl_qop_pmeasure : public impl {
    public:
        impl_qop_pmeasure(VariationalQuantumCircuit,
            std::vector<size_t>,
            QuantumMachine*,
            std::vector<Qubit*>);

        std::vector<double> _get_gradient(var _var);
        std::vector<double> _get_value();
        std::vector<double> _get_circuit_value(QCircuit);

    private:
        std::vector<Qubit*> m_measure_qubits;
        std::vector<size_t> m_components;
        QuantumMachine* m_machine;
        VariationalQuantumCircuit m_circuit;
    };

    struct impl_qop_pmeasure_real_chip : public impl {
    public:
        impl_qop_pmeasure_real_chip(VariationalQuantumCircuit,
            std::vector<size_t>,
            QuantumMachine*,
            std::vector<Qubit*>,
            std::vector<ClassicalCondition>,
            size_t shots);

        std::vector<double> _get_gradient(var _var);
        std::vector<double> _get_value();
        std::vector<double> _get_circuit_value(QCircuit);

    private:
        size_t m_shots;
        std::vector<Qubit*> m_measure_qubits;
        std::vector<ClassicalCondition> m_cbits;
        std::vector<size_t> m_components;
        QuantumMachine* m_machine;
        VariationalQuantumCircuit m_circuit;
    };

    // Inline definitions of templated functions:
    template <typename... V>
    const var pack_expression(op_type op, V&... args) {
        std::vector<std::shared_ptr<impl> > vimpl = { args.pimpl... };
        std::vector<var> v;
        for (const std::shared_ptr<impl>& _impl : vimpl) {
            v.emplace_back(_impl);
        }
        var res(op, v);
        for (const std::shared_ptr<impl>& _impl : vimpl) {
            _impl->parents.push_back(res.pimpl);
        }
        return res;
    }

    template <typename... V>
    const var pack_expression(op_type op, int axis, V&... args) {
        std::vector<std::shared_ptr<impl> > vimpl = { args.pimpl... };
        std::vector<var> v;
        for (const std::shared_ptr<impl>& _impl : vimpl) {
            v.emplace_back(_impl);
        }
        var res(std::make_shared<impl_stack>(axis, v));
        for (const std::shared_ptr<impl>& _impl : vimpl) {
            _impl->parents.push_back(res.pimpl);
        }
        return res;
    }

    // We need const-ness in returns here to prevent things like:
    // a + b = c; which is obviously dumb

    inline const var operator+(var lhs, var rhs) {
        return pack_expression(op_type::plus, lhs, rhs);
    }

    inline const var operator-(var lhs, var rhs) {
        return pack_expression(op_type::minus, lhs, rhs);
    }

    inline const var operator*(var lhs, var rhs) {
        return pack_expression(op_type::multiply, lhs, rhs);
    }

    inline const var operator/(var lhs, var rhs) {
        return pack_expression(op_type::divide, lhs, rhs);
    }

    inline const var exp(var v) {
        return pack_expression(op_type::exponent, v);
    }
    inline const var sigmoid(var v) {
        return pack_expression(op_type::sigmoid, v);
    }
    inline const var log(var v) {
        return pack_expression(op_type::log, v);
    }

    inline const var poly(var v, var power) {
        return pack_expression(op_type::polynomial, v, power);
    }

    inline const var dot(var lhs, var rhs) {
        return pack_expression(op_type::dot, lhs, rhs);
    }

    inline const var inverse(var v) {
        return pack_expression(op_type::inverse, v);
    }

    inline const var transpose(var v) {
        return pack_expression(op_type::transpose, v);
    }

    inline const var sum(var v) {
        return pack_expression(op_type::sum, v);
    }

    inline const var softmax(var v) {
        return pack_expression(op_type::softmax, v);
    }

    inline const var crossEntropy(var lhs, var rhs) {
        return pack_expression(op_type::cross_entropy, lhs, rhs);
    }

    inline const var dropout(var lhs, var rhs) {
        return pack_expression(op_type::dropout, lhs, rhs);
    }
    template <typename ...T>
    inline const var stack(int axis, T&... v) {
        return pack_expression(op_type::stack, axis, v...);
    }

    // QOP Functions
    //                            SingleAmp    PartialAmp     FullAmp     RealChip     NoisyFullAmp    Cloud      Impl        UseShots?
    // QOP                     |     N      |     N        |     Y     |     N      |       N        |   N    |    PMeasure |     N     |
    // QOP_PMEASURE            |     Y      |     Y        |     N     |     N      |       N        |   N    |    PMeasure |     N     |
    // QOP_REAL_CHIP           |     N      |     N        |     N     |     Y      |       Y        |   Y    |    Run      |     Y     |
    // QOP_PMEASURE_REAL_CHIP  |     N      |     N        |     N     |     Y      |       Y        |   Y    |    Run      |     Y     |

    inline const var qop(VariationalQuantumCircuit& circuit,
        PauliOperator Hamiltonian,
        QuantumMachine* machine,
        std::vector<Qubit*> measure_qubits)
    {
        auto pimpl = std::make_shared<impl_vqp>(circuit, Hamiltonian, machine, measure_qubits);
        var res(pimpl);
        std::vector<var>& vars = circuit.get_vars();
        for (auto &var : vars)
        {
            var.pimpl->parents.push_back(res.pimpl);
        }
        return res;
    }

    inline const var qop_real_chip(VariationalQuantumCircuit& circuit,
        PauliOperator Hamiltonian,
        QuantumMachine* machine,
        std::vector<Qubit*> measure_qubits,
        int shots)
    {
        auto pimpl = std::make_shared<impl_vqp_real_chip>(circuit, Hamiltonian, machine, measure_qubits, shots);
        var res(pimpl);
        std::vector<var>& vars = circuit.get_vars();
        for (auto &var : vars)
        {
            var.pimpl->parents.push_back(res.pimpl);
        }
        return res;
    }

    inline const var qop(VariationalQuantumCircuit& circuit,
        PauliOperator Hamiltonian,
        QuantumMachine* machine,
        std::map<size_t, Qubit*> measure_qubits)
    {
        auto pimpl = std::make_shared<impl_vqp>(circuit, Hamiltonian, machine, measure_qubits);
        var res(pimpl);
        std::vector<var>& vars = circuit.get_vars();
        for (auto &var : vars)
        {
            var.pimpl->parents.push_back(res.pimpl);
        }
        return res;
    }

    inline const var qop_pmeasure(VariationalQuantumCircuit& circuit,
        std::vector<size_t> components,
        QuantumMachine* machine,
        std::vector<Qubit*> measure_qubits)
    {
        auto pimpl = std::make_shared<impl_qop_pmeasure>(circuit, components, machine, measure_qubits);
        var res(pimpl);
        std::vector<var>& vars = circuit.get_vars();
        for (auto &var : vars)
        {
            var.pimpl->parents.push_back(res.pimpl);
        }
        return res;
    }

    inline const var qop_pmeasure_real_chip(VariationalQuantumCircuit& circuit,
        std::vector<size_t> components,
        QuantumMachine* machine,
        std::vector<Qubit*> measure_qubits,
        std::vector<ClassicalCondition> cbits,
        size_t shots)
    {
        auto pimpl = std::make_shared<impl_qop_pmeasure_real_chip>(circuit, components, machine, measure_qubits, cbits, shots);
        var res(pimpl);
        std::vector<var>& vars = circuit.get_vars();
        for (auto &var : vars)
        {
            var.pimpl->parents.push_back(res.pimpl);
        }
        return res;
    }

    inline bool _is_scalar(const var& v) { return v.getValue().size() == 1; }
    inline bool _is_matrix(const var& v) { return v.getValue().cols() > 1 && v.getValue().rows() > 1; }
    inline bool _is_vector(const var& v) { return (v.getValue().cols() == 1) ^ (v.getValue().rows() == 1); }
    inline double _sval(const var& v) { return v.getValue()(0, 0); }
    inline MatrixXd _mval(const var& v) { return v.getValue(); }

    inline MatrixXd scalar(double num) {
        MatrixXd m(1, 1);
        m(0, 0) = num;
        return m;
    }

    inline MatrixXd vector2mat(std::vector<double> data) {
        MatrixXd m(1, data.size());
        for (size_t i = 0; i < data.size(); i++)
            m(0, i) = data[i];
        return m;
    }

    inline MatrixXd zeros_like(const MatrixXd& like) {
        return MatrixXd::Zero(like.rows(), like.cols());
    }

    inline MatrixXd zeros_like(const var& like) {
        return MatrixXd::Zero(like.getValue().rows(), like.getValue().cols());
    }

    inline MatrixXd ones_like(const MatrixXd& like) {
        return MatrixXd::Ones(like.rows(), like.cols());
    }

    inline MatrixXd ones_like(const var& like) {
        return MatrixXd::Ones(like.getValue().rows(), like.getValue().cols());
    }

} // namespace Variational

QPANDA_END


#endif // ! VAR_H
