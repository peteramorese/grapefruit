#pragma once

#include <Eigen/Dense>

#define GF_CHECK_MATRIX_POS_DEF

#ifdef GF_CHECK_MATRIX_POS_DEF
    #define GF_IS_MATRIX_POS_SEMI_DEF(matrix) GF::isMatrixPositiveSemiDef(matrix)
    #define GF_IS_COV_POS_SEMI_DEF(matrix) GF::isCovariancePositiveSemiDef(matrix)
#else
    #define GF_IS_MATRIX_POS_SEMI_DEF(matrix) true
    #define GF_IS_COV_POS_SEMI_DEF(matrix) true
#endif

namespace GF {

    template <typename T>
    inline T abs(const T& x) {return (x < T{}) ? -x : x;}

    template <typename T>
    inline T diff(const T& lhs, const T& rhs) {
        if (lhs >= rhs)
            return lhs - rhs;
        else
            return rhs - lhs;
    }

    template <typename T>
    inline T max(const T& lhs, const T& rhs) {
        if (lhs >= rhs)
            return lhs;
        else 
            return rhs;
    }

    template <typename T>
    inline T min(const T& lhs, const T& rhs) {
        if (lhs <= rhs)
            return lhs;
        else 
            return rhs;
    }

    std::string templateToLabel(std::string label_template, uint32_t num, char delimeter = '#') {
        uint32_t i = 0;
        while (i < label_template.size()) {
            if (label_template[i] == delimeter) {
                label_template.replace(i, 1, std::to_string(num));
            }
            ++i;
        }
        return label_template;
    }

#ifdef GF_CHECK_MATRIX_POS_DEF
    template <class MATRIX_T>
    static bool isMatrixPositiveSemiDef(const MATRIX_T& matrix) {
        if (!matrix.isApprox(matrix.transpose()))
            return false;
        const auto ldlt = matrix.template selfadjointView<Eigen::Upper>().ldlt();
        if (ldlt.info() == Eigen::NumericalIssue || !ldlt.isPositive())
            return false;
        return true;
    }

    template <class MATRIX_T>
    static bool isCovariancePositiveSemiDef(const MATRIX_T& covariance) {
        const auto ldlt = covariance.template selfadjointView<Eigen::Upper>().ldlt();
        if (ldlt.info() == Eigen::NumericalIssue || !ldlt.isPositive())
            return false;
        return true;
    }
#endif

}
