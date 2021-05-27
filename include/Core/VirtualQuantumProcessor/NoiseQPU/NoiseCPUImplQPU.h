/*
Copyright (c) 2017-2020 Origin Quantum Computing. All Right Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#ifndef NOISY_CPU_QUANTUM_GATE_SINGLE_THREAD_H
#define NOISY_CPU_QUANTUM_GATE_SINGLE_THREAD_H

#include "Core/VirtualQuantumProcessor/QPUImpl.h"
#include "Core/VirtualQuantumProcessor/NoiseQPU/NoiseModel.h"
#include "ThirdParty/rapidjson/document.h"
#include <stdio.h>
#include <iostream>
#include <vector>


#ifndef SQ2
#define SQ2 (1 / 1.4142135623731)
#endif

#ifndef PI
#define PI 3.14159265358979
#endif

//#define NoiseOp std::vector<std::vector<qcomplex_t>>

QStat matrix_multiply(const QStat &matrix_left, const QStat &matrix_right);


/**
* @brief QPU implementation by noisy CPU model
* @ingroup VirtualQuantumProcessor
*/
class NoisyCPUImplQPU : public QPUImpl
{
public:
    vQParam qubit2stat;
    vQParam init_qubit2stat;
    QGateParam &findgroup(size_t qn);
    NoisyCPUImplQPU();
    NoisyCPUImplQPU(rapidjson::Document &);
    NoisyCPUImplQPU(NoisyQuantum &quantum_noise);
    void set_quantum_noise(const NoisyQuantum &quantum_noise);
    ~NoisyCPUImplQPU();

    bool TensorProduct(QGateParam& qgroup0, QGateParam& qgroup1);
    QError unitarySingleQubitGate(size_t qn,
        QStat& matrix,
        bool isConjugate,
        GateType type);
    QError noisyUnitarySingleQubitGate(size_t qn,
        QStat& matrix,
        bool isConjugate,
        NoiseOp & noise);

    QError controlunitarySingleQubitGate(size_t qn,
        Qnum& vControlBit,
        QStat& matrix,
        bool isConjugate,
        GateType type);

    QError unitaryDoubleQubitGate(size_t qn_0,
        size_t qn_1,
        QStat& matrix,
        bool isConjugate,
        GateType type);

    QError noisyUnitaryDoubleQubitGate(size_t qn_0,
        size_t qn_1,
        QStat& matrix,
        bool isConjugate,
        NoiseOp & noise);

    QError controlunitaryDoubleQubitGate(size_t qn_0,
        size_t qn_1,
        Qnum& vControlBit,
        QStat& matrix,
        bool isConjugate,
        GateType type);

    virtual QError Hadamard(size_t qn, bool isConjugate,
        double error_rate);

    virtual QError Hadamard(size_t qn, Qnum& vControlBit,
        bool isConjugate, double error_rate);

    virtual QError X(size_t qn, bool isConjugate,
        double error_rate);

    virtual QError X(size_t qn, Qnum& vControlBit,
        bool isConjugate, double error_rate);

    virtual QError P0(size_t qn, bool isConjugate,
        double error_rate);

    virtual QError P0(size_t qn, Qnum& vControlBit,
        bool isConjugate, double error_rate);

    virtual QError P1(size_t qn, bool isConjugate,
        double error_rate);

    virtual QError P1(size_t qn, Qnum& vControlBit,
        bool isConjugate, double error_rate);

    virtual QError Y(size_t qn, bool isConjugate,
        double error_rate);

    virtual QError Y(size_t qn, Qnum& vControlBit,
        bool isConjugate, double error_rate);

    virtual QError Z(size_t qn, bool isConjugate,
        double error_rate);

    virtual QError Z(size_t qn, Qnum& vControlBit,
        bool isConjugate, double error_rate);

    virtual QError T(size_t qn, bool isConjugate,
        double error_rate);

    virtual QError T(size_t qn, Qnum& vControlBit,
        bool isConjugate, double error_rate);

    virtual QError S(size_t qn, bool isConjugate,
        double error_rate);

    virtual QError S(size_t qn, Qnum& vControlBit,
        bool isConjugate, double error_rate);

    virtual QError U1_GATE(size_t qn, double theta,
        bool isConjugate, double error_rate);

    virtual QError RX_GATE(size_t qn, double theta,
        bool isConjugate, double error_rate);

    virtual QError RX_GATE(size_t qn, double theta,
        Qnum& vControlBit,
        bool isConjugate,
        double error_rate);

    virtual QError RY_GATE(size_t qn, double theta,
        bool isConjugate, double error_rate);

    virtual QError RY_GATE(size_t qn, double theta,
        Qnum& vControlBit,
        bool isConjugate,
        double error_rate);

    virtual QError RZ_GATE(size_t qn, double theta,
        bool isConjugate, double error_rate);

    virtual QError RZ_GATE(size_t qn, double theta,
        Qnum& vControlBit,
        bool isConjugate,
        double error_rate);

    virtual QError CNOT(size_t qn_0, size_t qn_1,
        bool isConjugate, double error_rate);

    virtual QError CNOT(size_t qn_0, size_t qn_1,
        Qnum& vControlBit,
        bool isConjugate,
        double error_rate);

    virtual QError CR(size_t qn_0, size_t qn_1, double theta,
        bool isConjugate, double error_rate);

    virtual QError CR(size_t qn_0, size_t qn_1,
        Qnum& vControlBit,
        double theta,
        bool isConjugate,
        double error_rate);

    virtual QError CZ(size_t qn_0, size_t qn_1,
        bool isConjugate, double error_rate);

    virtual QError CZ(size_t qn_0, size_t qn_1,
        Qnum& vControlBit,
        bool isConjugate,
        double error_rate);

    virtual QError iSWAP(size_t qn_0, size_t qn_1,
        double theta,
        bool isConjugate,
        double error_rate);

    virtual QError iSWAP(size_t qn_0, size_t qn_1,
        Qnum& vControlBit,
        double theta,
        bool isConjugate,
        double error_rate);

    virtual QError iSWAP(size_t qn_0, size_t qn_1,
        bool isConjugate,
        double error_rate);

    virtual QError iSWAP(size_t qn_0, size_t qn_1,
        Qnum& vControlBit,
        bool isConjugate,
        double error_rate);

    virtual QError SqiSWAP(size_t qn_0, size_t qn_1,
        bool isConjugate,
        double error_rate);

    virtual QError SqiSWAP(size_t qn_0, size_t qn_1,
        Qnum& vControlBit,
        bool isConjugate,
        double error_rate);
    virtual QError DiagonalGate(Qnum& vQubit,
        QStat & matrix,
        bool isConjugate,
        double error_rate);
    virtual QError controlDiagonalGate(Qnum& vQubit,
        QStat & matrix,
        Qnum& vControlBit,
        bool isConjugate,
        double error_rate);

    QStat getQState();
    QError Reset(size_t qn);
    bool qubitMeasure(size_t qn);
    QError pMeasure(Qnum& qnum, prob_tuple &mResult, int select_max=-1);
    QError pMeasure(Qnum& qnum, prob_vec &mResult);
    QError initState(size_t head_rank, size_t rank_size, size_t qubit_num);
    QError initState(size_t qubit_num, const QStat &state = {});

    // new noise modle
    QError unitary_noise_qubit_gate(const Qnum &qns, const QStat& matrix,
                                    bool is_conjugate, GateType type);
    QError unitary_noise_qubit_kraus(const Qnum & qns, const NoiseOp & ops, QStat & standard_matrix);

    QError unitary_qubit_gate_standard(size_t qn, QStat& matrix,
                                       bool is_conjugate);
    QError unitary_qubit_gate_standard(size_t qn0 ,size_t qn1, QStat& matrix,
                                       bool is_conjugate);
    QError reset_standard(size_t qn);
    bool measure_standard(size_t qn);
    bool readout(bool measure, size_t qn);

    void normlize(QStat &matrix, double p);
    double unitary_kraus(const Qnum &qns, const QStat &op);
private:
    QError singleQubitGateNoise(size_t qn, NoiseOp &noise);
    QError doubleQubitGateNoise(size_t qn_0, size_t qn_1, NoiseOp &noise);
    QError _get_probabilities(prob_vec &probabilities,
        size_t qn, NoiseOp & noise);
    QError _get_probabilities(prob_vec &probabilities,
        size_t qn_0, size_t qn_1, NoiseOp & noise);

    rapidjson::Document m_doc;

    // new noise model
    NoisyQuantum m_quantum_noise;
    RandomEngine19937 m_rng;
};

#endif // ! NOISY_CPU_QUANTUM_GATE_SINGLE_THREAD_H
