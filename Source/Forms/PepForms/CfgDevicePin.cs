/***************************************************************************/
/*  Copyright (C) 2009-2014 Kevin Eshbach                                  */
/***************************************************************************/

namespace Pep
{
    namespace Forms
    {
        public class CfgDevicePin
        {
            private System.UInt32 m_nPinNumber = 0;
            private System.String[] m_sTypes = null;
            private System.String[] m_sPolarities = null;
            private System.String m_sSelectedType = "";
            private System.String m_sSelectedPolarity = "";

            public System.UInt32 PinNumber
            {
                get
                {
                    return m_nPinNumber;
                }

                set
                {
                    m_nPinNumber = value;
                }
            }

            public System.String[] Types
            {
                get
                {
                    return m_sTypes;
                }

                set
                {
                    m_sTypes = value;
                }
            }

            public System.String[] Polarities
            {
                get
                {
                    return m_sPolarities;
                }

                set
                {
                    m_sPolarities = value;
                }
            }

            public System.String SelectedType
            {
                get
                {
                    return m_sSelectedType;
                }

                set
                {
                    m_sSelectedType = value;
                }
            }

            public System.String SelectedPolarity
            {
                get
                {
                    return m_sSelectedPolarity;
                }

                set
                {
                    m_sSelectedPolarity = value;
                }
            }

            public CfgDevicePin(
              System.UInt32 nPinNumber,
              System.String[] sTypes,
              System.String[] sPolarities)
            {
                m_nPinNumber = nPinNumber;
                m_sTypes = sTypes;
                m_sPolarities = sPolarities;
            }

            public CfgDevicePin(
              System.UInt32 nPinNumber,
              System.String[] sTypes,
              System.String sSelectedType,
              System.String[] sPolarities,
              System.String sSelectedPolarity)
            {
                m_nPinNumber = nPinNumber;
                m_sTypes = sTypes;
                m_sSelectedType = sSelectedType;
                m_sPolarities = sPolarities;
                m_sSelectedPolarity = sSelectedPolarity;
            }
        }
    }
}

/***************************************************************************/
/*  Copyright (C) 2009-2014 Kevin Eshbach                                  */
/***************************************************************************/
