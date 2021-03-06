// Boost.Geometry - gis-projections (based on PROJ4)

// Copyright (c) 2008-2015 Barend Gehrels, Amsterdam, the Netherlands.

// This file was modified by Oracle on 2017, 2018.
// Modifications copyright (c) 2017-2018, Oracle and/or its affiliates.
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// This file is converted from PROJ4, http://trac.osgeo.org/proj
// PROJ4 is originally written by Gerald Evenden (then of the USGS)
// PROJ4 is maintained by Frank Warmerdam
// PROJ4 is converted to Boost.Geometry by Barend Gehrels

// Last updated version of proj: 5.0.0

// Original copyright notice:

// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.

#ifndef BOOST_GEOMETRY_PROJECTIONS_SCONICS_HPP
#define BOOST_GEOMETRY_PROJECTIONS_SCONICS_HPP


#include <boost/geometry/util/math.hpp>
#include <boost/math/special_functions/hypot.hpp>

#include <boost/geometry/srs/projections/impl/base_static.hpp>
#include <boost/geometry/srs/projections/impl/base_dynamic.hpp>
#include <boost/geometry/srs/projections/impl/projects.hpp>
#include <boost/geometry/srs/projections/impl/factory_entry.hpp>

namespace boost { namespace geometry
{

namespace srs { namespace par4
{
    struct euler {};  // Euler
    struct murd1 {};  // Murdoch I
    struct murd2 {};  // Murdoch II
    struct murd3 {};  // Murdoch III
    struct pconic {}; // Perspective Conic
    struct tissot {}; // Tissot
    struct vitk1 {};  // Vitkovsky I

}} //namespace srs::par4

namespace projections
{
    #ifndef DOXYGEN_NO_DETAIL
    namespace detail { namespace sconics
    {

            enum Type {
                EULER  = 0,
                MURD1  = 1,
                MURD2  = 2,
                MURD3  = 3,
                PCONIC = 4,
                TISSOT = 5,
                VITK1  = 6
            };
            static const double EPS10 = 1.e-10;
            static const double EPS = 1e-10;

            template <typename T>
            struct par_sconics
            {
                T   n;
                T   rho_c;
                T   rho_0;
                T   sig;
                T   c1, c2;
                enum Type type;
            };

            /* get common factors for simple conics */
            template <typename Parameters, typename T>
            inline int phi12(Parameters& par, par_sconics<T>& proj_parm, T *del)
            {
                T p1, p2;
                int err = 0;

                if (!pj_param(par.params, "tlat_1").i ||
                    !pj_param(par.params, "tlat_2").i) {
                    err = -41;
                } else {
                    p1 = pj_param(par.params, "rlat_1").f;
                    p2 = pj_param(par.params, "rlat_2").f;
                    *del = 0.5 * (p2 - p1);
                    proj_parm.sig = 0.5 * (p2 + p1);
                    err = (fabs(*del) < EPS || fabs(proj_parm.sig) < EPS) ? -42 : 0;
                }
                return err;
            }

            // template class, using CRTP to implement forward/inverse
            template <typename CalculationType, typename Parameters>
            struct base_sconics_spheroid : public base_t_fi<base_sconics_spheroid<CalculationType, Parameters>,
                     CalculationType, Parameters>
            {

                typedef CalculationType geographic_type;
                typedef CalculationType cartesian_type;

                par_sconics<CalculationType> m_proj_parm;

                inline base_sconics_spheroid(const Parameters& par)
                    : base_t_fi<base_sconics_spheroid<CalculationType, Parameters>,
                     CalculationType, Parameters>(*this, par) {}

                // FORWARD(s_forward)  spheroid
                // Project coordinates from geographic (lon, lat) to cartesian (x, y)
                inline void fwd(geographic_type& lp_lon, geographic_type& lp_lat, cartesian_type& xy_x, cartesian_type& xy_y) const
                {
                    CalculationType rho;

                    switch (this->m_proj_parm.type) {
                    case MURD2:
                        rho = this->m_proj_parm.rho_c + tan(this->m_proj_parm.sig - lp_lat);
                        break;
                    case PCONIC:
                        rho = this->m_proj_parm.c2 * (this->m_proj_parm.c1 - tan(lp_lat - this->m_proj_parm.sig));
                        break;
                    default:
                        rho = this->m_proj_parm.rho_c - lp_lat;
                        break;
                    }
                    xy_x = rho * sin( lp_lon *= this->m_proj_parm.n );
                    xy_y = this->m_proj_parm.rho_0 - rho * cos(lp_lon);
                }

                // INVERSE(s_inverse)  ellipsoid & spheroid
                // Project coordinates from cartesian (x, y) to geographic (lon, lat)
                inline void inv(cartesian_type& xy_x, cartesian_type& xy_y, geographic_type& lp_lon, geographic_type& lp_lat) const
                {
                    CalculationType rho;

                    rho = boost::math::hypot(xy_x, xy_y = this->m_proj_parm.rho_0 - xy_y);
                    if (this->m_proj_parm.n < 0.) {
                        rho = - rho;
                        xy_x = - xy_x;
                        xy_y = - xy_y;
                    }

                    lp_lon = atan2(xy_x, xy_y) / this->m_proj_parm.n;

                    switch (this->m_proj_parm.type) {
                    case PCONIC:
                        lp_lat = atan(this->m_proj_parm.c1 - rho / this->m_proj_parm.c2) + this->m_proj_parm.sig;
                        break;
                    case MURD2:
                        lp_lat = this->m_proj_parm.sig - atan(rho - this->m_proj_parm.rho_c);
                        break;
                    default:
                        lp_lat = this->m_proj_parm.rho_c - rho;
                    }
                }

                static inline std::string get_name()
                {
                    return "sconics_spheroid";
                }

            };

            template <typename Parameters, typename T>
            inline void setup(Parameters& par, par_sconics<T>& proj_parm, Type type) 
            {
                static const T HALFPI = detail::HALFPI<T>();

                T del, cs;
                int err;

                proj_parm.type = type;

                err = phi12(par, proj_parm, &del);
                if(err)
                    BOOST_THROW_EXCEPTION( projection_exception(err) );

                switch (proj_parm.type) {
                case TISSOT:
                    proj_parm.n = sin(proj_parm.sig);
                    cs = cos(del);
                    proj_parm.rho_c = proj_parm.n / cs + cs / proj_parm.n;
                    proj_parm.rho_0 = sqrt((proj_parm.rho_c - 2 * sin(par.phi0))/proj_parm.n);
                    break;
                case MURD1:
                    proj_parm.rho_c = sin(del)/(del * tan(proj_parm.sig)) + proj_parm.sig;
                    proj_parm.rho_0 = proj_parm.rho_c - par.phi0;
                    proj_parm.n = sin(proj_parm.sig);
                    break;
                case MURD2:
                    proj_parm.rho_c = (cs = sqrt(cos(del))) / tan(proj_parm.sig);
                    proj_parm.rho_0 = proj_parm.rho_c + tan(proj_parm.sig - par.phi0);
                    proj_parm.n = sin(proj_parm.sig) * cs;
                    break;
                case MURD3:
                    proj_parm.rho_c = del / (tan(proj_parm.sig) * tan(del)) + proj_parm.sig;
                    proj_parm.rho_0 = proj_parm.rho_c - par.phi0;
                    proj_parm.n = sin(proj_parm.sig) * sin(del) * tan(del) / (del * del);
                    break;
                case EULER:
                    proj_parm.n = sin(proj_parm.sig) * sin(del) / del;
                    del *= 0.5;
                    proj_parm.rho_c = del / (tan(del) * tan(proj_parm.sig)) + proj_parm.sig;
                    proj_parm.rho_0 = proj_parm.rho_c - par.phi0;
                    break;
                case PCONIC:
                    proj_parm.n = sin(proj_parm.sig);
                    proj_parm.c2 = cos(del);
                    proj_parm.c1 = 1./tan(proj_parm.sig);
                    if (fabs(del = par.phi0 - proj_parm.sig) - EPS10 >= HALFPI)
                        BOOST_THROW_EXCEPTION( projection_exception(-43) );
                    proj_parm.rho_0 = proj_parm.c2 * (proj_parm.c1 - tan(del));
                    break;
                case VITK1:
                    proj_parm.n = (cs = tan(del)) * sin(proj_parm.sig) / del;
                    proj_parm.rho_c = del / (cs * tan(proj_parm.sig)) + proj_parm.sig;
                    proj_parm.rho_0 = proj_parm.rho_c - par.phi0;
                    break;
                }

                par.es = 0;
            }


            // Euler
            template <typename Parameters, typename T>
            inline void setup_euler(Parameters& par, par_sconics<T>& proj_parm)
            {
                setup(par, proj_parm, EULER);
            }

            // Tissot
            template <typename Parameters, typename T>
            inline void setup_tissot(Parameters& par, par_sconics<T>& proj_parm)
            {
                setup(par, proj_parm, TISSOT);
            }

            // Murdoch I
            template <typename Parameters, typename T>
            inline void setup_murd1(Parameters& par, par_sconics<T>& proj_parm)
            {
                setup(par, proj_parm, MURD1);
            }

            // Murdoch II
            template <typename Parameters, typename T>
            inline void setup_murd2(Parameters& par, par_sconics<T>& proj_parm)
            {
                setup(par, proj_parm, MURD2);
            }

            // Murdoch III
            template <typename Parameters, typename T>
            inline void setup_murd3(Parameters& par, par_sconics<T>& proj_parm)
            {
                setup(par, proj_parm, MURD3);
            }            

            // Perspective Conic
            template <typename Parameters, typename T>
            inline void setup_pconic(Parameters& par, par_sconics<T>& proj_parm)
            {
                setup(par, proj_parm, PCONIC);
            }

            // Vitkovsky I
            template <typename Parameters, typename T>
            inline void setup_vitk1(Parameters& par, par_sconics<T>& proj_parm)
            {
                setup(par, proj_parm, VITK1);
            }

    }} // namespace detail::sconics
    #endif // doxygen
    
    /*!
        \brief Tissot projection
        \ingroup projections
        \tparam Geographic latlong point type
        \tparam Cartesian xy point type
        \tparam Parameters parameter type
        \par Projection characteristics
         - Conic
         - Spheroid
        \par Projection parameters
         - lat_1: Latitude of first standard parallel
         - lat_2: Latitude of second standard parallel
        \par Example
        \image html ex_tissot.gif
    */
    template <typename CalculationType, typename Parameters>
    struct tissot_spheroid : public detail::sconics::base_sconics_spheroid<CalculationType, Parameters>
    {
        inline tissot_spheroid(const Parameters& par) : detail::sconics::base_sconics_spheroid<CalculationType, Parameters>(par)
        {
            detail::sconics::setup_tissot(this->m_par, this->m_proj_parm);
        }
    };

    /*!
        \brief Murdoch I projection
        \ingroup projections
        \tparam Geographic latlong point type
        \tparam Cartesian xy point type
        \tparam Parameters parameter type
        \par Projection characteristics
         - Conic
         - Spheroid
        \par Projection parameters
         - lat_1: Latitude of first standard parallel
         - lat_2: Latitude of second standard parallel
        \par Example
        \image html ex_murd1.gif
    */
    template <typename CalculationType, typename Parameters>
    struct murd1_spheroid : public detail::sconics::base_sconics_spheroid<CalculationType, Parameters>
    {
        inline murd1_spheroid(const Parameters& par) : detail::sconics::base_sconics_spheroid<CalculationType, Parameters>(par)
        {
            detail::sconics::setup_murd1(this->m_par, this->m_proj_parm);
        }
    };

    /*!
        \brief Murdoch II projection
        \ingroup projections
        \tparam Geographic latlong point type
        \tparam Cartesian xy point type
        \tparam Parameters parameter type
        \par Projection characteristics
         - Conic
         - Spheroid
        \par Projection parameters
         - lat_1: Latitude of first standard parallel
         - lat_2: Latitude of second standard parallel
        \par Example
        \image html ex_murd2.gif
    */
    template <typename CalculationType, typename Parameters>
    struct murd2_spheroid : public detail::sconics::base_sconics_spheroid<CalculationType, Parameters>
    {
        inline murd2_spheroid(const Parameters& par) : detail::sconics::base_sconics_spheroid<CalculationType, Parameters>(par)
        {
            detail::sconics::setup_murd2(this->m_par, this->m_proj_parm);
        }
    };

    /*!
        \brief Murdoch III projection
        \ingroup projections
        \tparam Geographic latlong point type
        \tparam Cartesian xy point type
        \tparam Parameters parameter type
        \par Projection characteristics
         - Conic
         - Spheroid
        \par Projection parameters
         - lat_1: Latitude of first standard parallel
         - lat_2: Latitude of second standard parallel
        \par Example
        \image html ex_murd3.gif
    */
    template <typename CalculationType, typename Parameters>
    struct murd3_spheroid : public detail::sconics::base_sconics_spheroid<CalculationType, Parameters>
    {
        inline murd3_spheroid(const Parameters& par) : detail::sconics::base_sconics_spheroid<CalculationType, Parameters>(par)
        {
            detail::sconics::setup_murd3(this->m_par, this->m_proj_parm);
        }
    };

    /*!
        \brief Euler projection
        \ingroup projections
        \tparam Geographic latlong point type
        \tparam Cartesian xy point type
        \tparam Parameters parameter type
        \par Projection characteristics
         - Conic
         - Spheroid
        \par Projection parameters
         - lat_1: Latitude of first standard parallel
         - lat_2: Latitude of second standard parallel
        \par Example
        \image html ex_euler.gif
    */
    template <typename CalculationType, typename Parameters>
    struct euler_spheroid : public detail::sconics::base_sconics_spheroid<CalculationType, Parameters>
    {
        inline euler_spheroid(const Parameters& par) : detail::sconics::base_sconics_spheroid<CalculationType, Parameters>(par)
        {
            detail::sconics::setup_euler(this->m_par, this->m_proj_parm);
        }
    };

    /*!
        \brief Perspective Conic projection
        \ingroup projections
        \tparam Geographic latlong point type
        \tparam Cartesian xy point type
        \tparam Parameters parameter type
        \par Projection characteristics
         - Conic
         - Spheroid
        \par Projection parameters
         - lat_1: Latitude of first standard parallel
         - lat_2: Latitude of second standard parallel
        \par Example
        \image html ex_pconic.gif
    */
    template <typename CalculationType, typename Parameters>
    struct pconic_spheroid : public detail::sconics::base_sconics_spheroid<CalculationType, Parameters>
    {
        inline pconic_spheroid(const Parameters& par) : detail::sconics::base_sconics_spheroid<CalculationType, Parameters>(par)
        {
            detail::sconics::setup_pconic(this->m_par, this->m_proj_parm);
        }
    };

    /*!
        \brief Vitkovsky I projection
        \ingroup projections
        \tparam Geographic latlong point type
        \tparam Cartesian xy point type
        \tparam Parameters parameter type
        \par Projection characteristics
         - Conic
         - Spheroid
        \par Projection parameters
         - lat_1: Latitude of first standard parallel
         - lat_2: Latitude of second standard parallel
        \par Example
        \image html ex_vitk1.gif
    */
    template <typename CalculationType, typename Parameters>
    struct vitk1_spheroid : public detail::sconics::base_sconics_spheroid<CalculationType, Parameters>
    {
        inline vitk1_spheroid(const Parameters& par) : detail::sconics::base_sconics_spheroid<CalculationType, Parameters>(par)
        {
            detail::sconics::setup_vitk1(this->m_par, this->m_proj_parm);
        }
    };

    #ifndef DOXYGEN_NO_DETAIL
    namespace detail
    {

        // Static projection
        BOOST_GEOMETRY_PROJECTIONS_DETAIL_STATIC_PROJECTION(srs::par4::euler, euler_spheroid, euler_spheroid)
        BOOST_GEOMETRY_PROJECTIONS_DETAIL_STATIC_PROJECTION(srs::par4::murd1, murd1_spheroid, murd1_spheroid)
        BOOST_GEOMETRY_PROJECTIONS_DETAIL_STATIC_PROJECTION(srs::par4::murd2, murd2_spheroid, murd2_spheroid)
        BOOST_GEOMETRY_PROJECTIONS_DETAIL_STATIC_PROJECTION(srs::par4::murd3, murd3_spheroid, murd3_spheroid)
        BOOST_GEOMETRY_PROJECTIONS_DETAIL_STATIC_PROJECTION(srs::par4::pconic, pconic_spheroid, pconic_spheroid)
        BOOST_GEOMETRY_PROJECTIONS_DETAIL_STATIC_PROJECTION(srs::par4::tissot, tissot_spheroid, tissot_spheroid)
        BOOST_GEOMETRY_PROJECTIONS_DETAIL_STATIC_PROJECTION(srs::par4::vitk1, vitk1_spheroid, vitk1_spheroid)
        
        // Factory entry(s)
        template <typename CalculationType, typename Parameters>
        class tissot_entry : public detail::factory_entry<CalculationType, Parameters>
        {
            public :
                virtual base_v<CalculationType, Parameters>* create_new(const Parameters& par) const
                {
                    return new base_v_fi<tissot_spheroid<CalculationType, Parameters>, CalculationType, Parameters>(par);
                }
        };

        template <typename CalculationType, typename Parameters>
        class murd1_entry : public detail::factory_entry<CalculationType, Parameters>
        {
            public :
                virtual base_v<CalculationType, Parameters>* create_new(const Parameters& par) const
                {
                    return new base_v_fi<murd1_spheroid<CalculationType, Parameters>, CalculationType, Parameters>(par);
                }
        };

        template <typename CalculationType, typename Parameters>
        class murd2_entry : public detail::factory_entry<CalculationType, Parameters>
        {
            public :
                virtual base_v<CalculationType, Parameters>* create_new(const Parameters& par) const
                {
                    return new base_v_fi<murd2_spheroid<CalculationType, Parameters>, CalculationType, Parameters>(par);
                }
        };

        template <typename CalculationType, typename Parameters>
        class murd3_entry : public detail::factory_entry<CalculationType, Parameters>
        {
            public :
                virtual base_v<CalculationType, Parameters>* create_new(const Parameters& par) const
                {
                    return new base_v_fi<murd3_spheroid<CalculationType, Parameters>, CalculationType, Parameters>(par);
                }
        };

        template <typename CalculationType, typename Parameters>
        class euler_entry : public detail::factory_entry<CalculationType, Parameters>
        {
            public :
                virtual base_v<CalculationType, Parameters>* create_new(const Parameters& par) const
                {
                    return new base_v_fi<euler_spheroid<CalculationType, Parameters>, CalculationType, Parameters>(par);
                }
        };

        template <typename CalculationType, typename Parameters>
        class pconic_entry : public detail::factory_entry<CalculationType, Parameters>
        {
            public :
                virtual base_v<CalculationType, Parameters>* create_new(const Parameters& par) const
                {
                    return new base_v_fi<pconic_spheroid<CalculationType, Parameters>, CalculationType, Parameters>(par);
                }
        };

        template <typename CalculationType, typename Parameters>
        class vitk1_entry : public detail::factory_entry<CalculationType, Parameters>
        {
            public :
                virtual base_v<CalculationType, Parameters>* create_new(const Parameters& par) const
                {
                    return new base_v_fi<vitk1_spheroid<CalculationType, Parameters>, CalculationType, Parameters>(par);
                }
        };

        template <typename CalculationType, typename Parameters>
        inline void sconics_init(detail::base_factory<CalculationType, Parameters>& factory)
        {
            factory.add_to_factory("tissot", new tissot_entry<CalculationType, Parameters>);
            factory.add_to_factory("murd1", new murd1_entry<CalculationType, Parameters>);
            factory.add_to_factory("murd2", new murd2_entry<CalculationType, Parameters>);
            factory.add_to_factory("murd3", new murd3_entry<CalculationType, Parameters>);
            factory.add_to_factory("euler", new euler_entry<CalculationType, Parameters>);
            factory.add_to_factory("pconic", new pconic_entry<CalculationType, Parameters>);
            factory.add_to_factory("vitk1", new vitk1_entry<CalculationType, Parameters>);
        }

    } // namespace detail
    #endif // doxygen

} // namespace projections

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_PROJECTIONS_SCONICS_HPP

