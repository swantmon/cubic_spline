//
//  Created by Tobias Schwandt.
//  Copyright (c) 2012 Zebresel. All rights reserved.
//

#include "core/cubic_spline.h"

#include <algorithm>

namespace Core
{
namespace Math
{
    bool CCubicSpline::AddPoint(Float2& _rNewPoint)
    {
        // -----------------------------------------------------------------------------
        // check if point alread exist
        // -----------------------------------------------------------------------------
        float CheckXValue;

        for (unsigned int IndexOfPoint = 0; IndexOfPoint < m_Points.size(); ++IndexOfPoint)
        {
            CheckXValue = m_Points[IndexOfPoint][0];

            if (CheckXValue >= _rNewPoint[0])
            {
                if (CheckXValue == _rNewPoint[0])
                {
                    // -----------------------------------------------------------------------------
                    // point can not be added to spline because this point with 
                    // this x-value already exist
                    // -----------------------------------------------------------------------------
                    return false;
                }

                // -----------------------------------------------------------------------------
                // point can't be already inside
                // -----------------------------------------------------------------------------
                break;
            }
        }


        // -----------------------------------------------------------------------------
        // add point to list
        // -----------------------------------------------------------------------------        
        m_Points.push_back(_rNewPoint);
        
        // -----------------------------------------------------------------------------
        // Check some spline rules
        // -----------------------------------------------------------------------------
        float AddXValue = _rNewPoint[0];
        
        if (m_Points.size() == 1) 
        {
            m_MinX = AddXValue;
            m_MaxX = AddXValue;
        }
        
        bool NeedSort = true;
        if (m_MinX > AddXValue) 
        {
            m_MinX = AddXValue;
        }
        else if (m_MaxX < AddXValue) 
        {
            m_MaxX = AddXValue;
            NeedSort = false;
        }
        
        // -----------------------------------------------------------------------------
        // sort array because on wrong calculation
        // -----------------------------------------------------------------------------
        if (NeedSort == true) 
        {
            std::sort(m_Points.begin(), m_Points.end());
        }
        
        // -----------------------------------------------------------------------------
        // set dirty flag
        // -----------------------------------------------------------------------------
        m_Dirty = true;

        return true;
    }

    // -----------------------------------------------------------------------------

    bool CCubicSpline::RemovePointWithPosition(float _XPos)
    {
        for (unsigned int IndexOfPoint = 0; IndexOfPoint < m_Points.size(); ++IndexOfPoint)
        {
            if (m_Points[IndexOfPoint][0] == _XPos)
            {
                return RemovePointOnIndex(IndexOfPoint);
            }
        }

        return false;
    }

    // -----------------------------------------------------------------------------

    bool CCubicSpline::RemovePointOnIndex(unsigned int _Index)
    {
        if (_Index < m_Points.size())
        {
            m_Points.erase(m_Points.begin() + _Index);
            CPoints OldPoints = m_Points;

            m_Points.clear();

            for (unsigned int IndexOfPoint = 0; IndexOfPoint < OldPoints.size(); ++IndexOfPoint)
            {
                AddPoint(OldPoints[IndexOfPoint]);
            }

            return true;
        }

        return false;
    }

    // -----------------------------------------------------------------------------

    float CCubicSpline::Interpolate(float _XPos)
    {
        return Interp(_XPos);
    }
    
    // -----------------------------------------------------------------------------
    
    float CCubicSpline::MinX()
    {
        return m_MinX;
    }
    
    // -----------------------------------------------------------------------------
    
    float CCubicSpline::MaxX()
    {
        return m_MaxX;
    }

    // -----------------------------------------------------------------------------
    
    int CCubicSpline::SizeOfPoints()
    {
        return m_Points.size();
    }
    
    // -----------------------------------------------------------------------------

    void CCubicSpline::BuildCoef()
    {
        // -----------------------------------------------------------------------------
        // build coef
        // -----------------------------------------------------------------------------
        int n = m_Points.size();
        
        // -----------------------------------------------------------------------------
        // clear and resize "z"
        // -----------------------------------------------------------------------------
        m_Z.clear();
        m_Z.resize(m_Points.size());

        // -----------------------------------------------------------------------------
        // declare and allocate "h", "b", "u" and "v"
        // -----------------------------------------------------------------------------
        float* pH = new float[n];
        float* pB = new float[n];
        float* pU = new float[n];
        float* pV = new float[n];

        // -----------------------------------------------------------------------------
        // claculate "h" and "b"
        // -----------------------------------------------------------------------------
        for (int i = 0; i < n - 1; ++i)
        {
            pH[i] = m_Points[i + 1][0] - m_Points[i][0];
            pB[i] = (m_Points[i + 1][1] - m_Points[i][1]) / pH[i];
        }

        // -----------------------------------------------------------------------------
        // calculate "u" and "v"
        // -----------------------------------------------------------------------------
        pU[1] = 2 * (pH[0] + pH[1]);
        pV[1] = 6 * (pB[1] - pB[0]);

        for (int i = 2; i < n - 1; ++i)
        {
            pU[i] = 2 * (pH[i] + pH[i - 1]) - (pH[i - 1] * pH[i - 1] / pU[i - 1]);
            pV[i] = 6 * (pB[i] - pB[i - 1]) - (pH[i - 1] * pV[i - 1] / pU[i - 1]);
        }

        // -----------------------------------------------------------------------------
        // last step: calculate "z"
        // -----------------------------------------------------------------------------
        m_Z[n - 1] = 0;

        for (int i = n - 1 - 1; i >= 1; --i)
        {
            m_Z[i] = (pV[i] - pH[i] * m_Z[i + 1]) / pU[i];
        }

        m_Z[0] = 0;


        // -----------------------------------------------------------------------------
        // delete temporary arrays
        // -----------------------------------------------------------------------------
        delete[] pV;
        delete[] pU;
        delete[] pB;
        delete[] pH;
    }

    // -----------------------------------------------------------------------------

    float CCubicSpline::Interp(float _XPos)
    {
        int n = m_Points.size();
        if (n <= 1) return 0.0f;
        
        // -----------------------------------------------------------------------------
        // build coef if points has changed
        // -----------------------------------------------------------------------------
        if (m_Dirty) 
        {
            BuildCoef();
            m_Dirty = false;
        }
        
        // -----------------------------------------------------------------------------
        // evaluate spline
        // -----------------------------------------------------------------------------
        int i;
        float h;
        float tmp;

        // -----------------------------------------------------------------------------
        // find begining of spline where x-value is inside
        // -----------------------------------------------------------------------------
        for (i = n - 1; i >= 0 ; --i)
        {
            if (_XPos - m_Points[i][0] >= 0)
            {
                //if last x-value is found
                break;
            }
        }

        // -----------------------------------------------------------------------------
        // calculate y-value
        // -----------------------------------------------------------------------------
        h = m_Points[i + 1][0] - m_Points[i][0];

        tmp = (m_Z[i] / 2) + ( ( (_XPos - m_Points[i][0]) * (m_Z[i + 1] - m_Z[i]) ) / (6 * h) );
        tmp = -(h / 6) * (m_Z[i + 1] + 2 * m_Z[i]) + ((m_Points[i + 1][1] - m_Points[i][1]) / h) + ((_XPos - m_Points[i][0]) * tmp);

        return m_Points[i][1] + (_XPos - m_Points[i][0]) * tmp;
    }
}
}