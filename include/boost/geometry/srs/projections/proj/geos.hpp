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

// Copyright (c) 2004   Gerald I. Evenden
// Copyright (c) 2012   Martin Raspaud

// See also (section 4.4.3.2):
//   http://www.eumetsat.int/en/area4/msg/news/us_doc/cgms_03_26.pdf

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

#ifndef BOOST_GEOMETRY_PROJECTIONS_GEOS_HPP
#define BOOST_GEOMETRY_PROJECTIONS_GEOS_HPP

#include <boost/math/special_functions/hypot.hpp>

#include <boost/geometry/srs/projections/impl/base_static.hpp>
#include <boost/geometry/srs/projections/impl/base_dynamic.hpp>
#include <boost/geometry/srs/projections/impl/projects.hpp>
#include <boost/geometry/srs/projections/impl/factory_entry.hpp>

namespace boost { namespace geometry
{

namespace srs { namespace par4
{
    struct geos {}; // Geostationary Satellite View

}} //namespace srs::par4

namespace projections
{
    #ifndef DOXYGEN_NO_DETAIL
    namespace detail { namespace geos
    {
            template <typename T>
            struct par_geos
            {
                T           h;
                T           radius_p;
                T           radius_p2;
                T           radius_p_inv2;
                T           radius_g;
                T           radius_g_1;
                T           C;
                int         flip_axis;
            };

            // template class, using CRTP to implement forward/inverse
            template <typename CalculationType, typename Parameters>
            struct base_geos_ellipsoid : public base_t_fi<base_geos_ellipsoid<CalculationType, Parameters>,
                     CalculationType, Parameters>
            {

                typedef CalculationType geographic_type;
                typedef CalculationType cartesian_type;

                par_geos<CalculationType> m_proj_parm;

                inline base_geos_ellipsoid(const Parameters& par)
                    : base_t_fi<base_geos_ellipsoid<CalculationType, Parameters>,
                     CalculationType, Parameters>(*this, par) {}

                // FORWARD(e_forward)  ellipsoid
                // Project coordinates from geographic (lon, lat) to cartesian (x, y)
                inline void fwd(geographic_type& lp_lon, geographic_type& lp_lat, cartesian_type& xy_x, cartesian_type& xy_y) const
                {
                    CalculationType r, Vx, Vy, Vz, tmp;

                    /* Calculation of geocentric latitude. */
                    lp_lat = atan (this->m_proj_parm.radius_p2 * tan (lp_lat));
                
                    /* Calculation of the three components of the vector from satellite to
                    ** position on earth surface (lon,lat).*/
                    r = (this->m_proj_parm.radius_p) / boost::math::hypot(this->m_proj_parm.radius_p * cos (lp_lat), sin (lp_lat));
                    Vx = r * cos (lp_lon) * cos (lp_lat);
                    Vy = r * sin (lp_lon) * cos (lp_lat);
                    Vz = r * sin (lp_lat);

                    /* Check visibility. */
                    if (((this->m_proj_parm.radius_g - Vx) * Vx - Vy * Vy - Vz * Vz * this->m_proj_parm.radius_p_inv2) < 0.) {
                        BOOST_THROW_EXCEPTION( projection_exception(-20) );
                    }

                    /* Calculation based on view angles from satellite. */
                    tmp = this->m_proj_parm.radius_g - Vx;

                    if(this->m_proj_parm.flip_axis) {
                        xy_x = this->m_proj_parm.radius_g_1 * atan (Vy / boost::math::hypot (Vz, tmp));
                        xy_y = this->m_proj_parm.radius_g_1 * atan (Vz / tmp);
                    } else {
                        xy_x = this->m_proj_parm.radius_g_1 * atan (Vy / tmp);
                        xy_y = this->m_proj_parm.radius_g_1 * atan (Vz / boost::math::hypot (Vy, tmp));
                    }
                }

                // INVERSE(e_inverse)  ellipsoid
                // Project coordinates from cartesian (x, y) to geographic (lon, lat)
                inline void inv(cartesian_type& xy_x, cartesian_type& xy_y, geographic_type& lp_lon, geographic_type& lp_lat) const
                {
                    CalculationType Vx, Vy, Vz, a, b, det, k;

                    /* Setting three components of vector from satellite to position.*/
                    Vx = -1.0;
                        
                    if(this->m_proj_parm.flip_axis) {
                        Vz = tan (xy_y / this->m_proj_parm.radius_g_1);
                        Vy = tan (xy_x / this->m_proj_parm.radius_g_1) * boost::math::hypot(1.0, Vz);
                    } else {
                        Vy = tan (xy_x / this->m_proj_parm.radius_g_1);
                        Vz = tan (xy_y / this->m_proj_parm.radius_g_1) * boost::math::hypot(1.0, Vy);
                    }

                    /* Calculation of terms in cubic equation and determinant.*/
                    a = Vz / this->m_proj_parm.radius_p;
                    a   = Vy * Vy + a * a + Vx * Vx;
                    b   = 2 * this->m_proj_parm.radius_g * Vx;
                    if ((det = (b * b) - 4 * a * this->m_proj_parm.C) < 0.) {
                        BOOST_THROW_EXCEPTION( projection_exception(-20) );
                    }

                    /* Calculation of three components of vector from satellite to position.*/
                    k  = (-b - sqrt(det)) / (2. * a);
                    Vx = this->m_proj_parm.radius_g + k * Vx;
                    Vy *= k;
                    Vz *= k;

                    /* Calculation of longitude and latitude.*/
                    lp_lon = atan2 (Vy, Vx);
                    lp_lat = atan (Vz * cos (lp_lon) / Vx);
                    lp_lat = atan (this->m_proj_parm.radius_p_inv2 * tan (lp_lat));
                }

                static inline std::string get_name()
                {
                    return "geos_ellipsoid";
                }

            };

            // template class, using CRTP to implement forward/inverse
            template <typename CalculationType, typename Parameters>
            struct base_geos_spheroid : public base_t_fi<base_geos_spheroid<CalculationType, Parameters>,
                     CalculationType, Parameters>
            {

                typedef CalculationType geographic_type;
                typedef CalculationType cartesian_type;

                par_geos<CalculationType> m_proj_parm;

                inline base_geos_spheroid(const Parameters& par)
                    : base_t_fi<base_geos_spheroid<CalculationType, Parameters>,
                     CalculationType, Parameters>(*this, par) {}

                // FORWARD(s_forward)  spheroid
                // Project coordinates from geographic (lon, lat) to cartesian (x, y)
                inline void fwd(geographic_type& lp_lon, geographic_type& lp_lat, cartesian_type& xy_x, cartesian_type& xy_y) const
                {
                    CalculationType Vx, Vy, Vz, tmp;

                    /* Calculation of the three components of the vector from satellite to
                    ** position on earth surface (lon,lat).*/
                    tmp = cos(lp_lat);
                    Vx = cos (lp_lon) * tmp;
                    Vy = sin (lp_lon) * tmp;
                    Vz = sin (lp_lat);

                    /* Check visibility.*/
                    // TODO: in proj4 5.0.0 this check is not present
                    if (((this->m_proj_parm.radius_g - Vx) * Vx - Vy * Vy - Vz * Vz) < 0.)
                        BOOST_THROW_EXCEPTION( projection_exception(-20) );

                    /* Calculation based on view angles from satellite.*/
                    tmp = this->m_proj_parm.radius_g - Vx;

                    if(this->m_proj_parm.flip_axis) {
                        xy_x = this->m_proj_parm.radius_g_1 * atan(Vy / boost::math::hypot(Vz, tmp));
                        xy_y = this->m_proj_parm.radius_g_1 * atan(Vz / tmp);
                    } else {
                        xy_x = this->m_proj_parm.radius_g_1 * atan(Vy / tmp);
                        xy_y = this->m_proj_parm.radius_g_1 * atan(Vz / boost::math::hypot(Vy, tmp));
                    }
                }

                // INVERSE(s_inverse)  spheroid
                // Project coordinates from cartesian (x, y) to geographic (lon, lat)
                inline void inv(cartesian_type& xy_x, cartesian_type& xy_y, geographic_type& lp_lon, geographic_type& lp_lat) const
                {
                    CalculationType Vx, Vy, Vz, a, b, det, k;

                    /* Setting three components of vector from satellite to position.*/
                    Vx = -1.0;
                    if(this->m_proj_parm.flip_axis) {
                        Vz = tan (xy_y / (this->m_proj_parm.radius_g - 1.0));
                        Vy = tan (xy_x / (this->m_proj_parm.radius_g - 1.0)) * sqrt (1.0 + Vz * Vz);
                    } else {
                        Vy = tan (xy_x / (this->m_proj_parm.radius_g - 1.0));
                        Vz = tan (xy_y / (this->m_proj_parm.radius_g - 1.0)) * sqrt (1.0 + Vy * Vy);
                    }
                    
                    /* Calculation of terms in cubic equation and determinant.*/
                    a   = Vy * Vy + Vz * Vz + Vx * Vx;
                    b   = 2 * this->m_proj_parm.radius_g * Vx;
                    if ((det = (b * b) - 4 * a * this->m_proj_parm.C) < 0.) {
                        BOOST_THROW_EXCEPTION( projection_exception(-20) );
                    }

                    /* Calculation of three components of vector from satellite to position.*/
                    k  = (-b - sqrt(det)) / (2 * a);
                    Vx = this->m_proj_parm.radius_g + k * Vx;
                    Vy *= k;
                    Vz *= k;

                    /* Calculation of longitude and latitude.*/
                    lp_lon = atan2 (Vy, Vx);
                    lp_lat = atan (Vz * cos (lp_lon) / Vx);
                }

                static inline std::string get_name()
                {
                    return "geos_spheroid";
                }

            };

            // Geostationary Satellite View
            template <typename Parameters, typename T>
            inline void setup_geos(Parameters& par, par_geos<T>& proj_parm)
            {
                std::string sweep_axis;

                if ((proj_parm.h = pj_param(par.params, "dh").f) <= 0.)
                    BOOST_THROW_EXCEPTION( projection_exception(-30) );

                if (par.phi0 != 0.0)
                    BOOST_THROW_EXCEPTION( projection_exception(-46) );

                sweep_axis = pj_param(par.params, "ssweep").s;
                if (sweep_axis.empty())
                    proj_parm.flip_axis = 0;
                else {
                    if (sweep_axis[1] != '\0' || (sweep_axis[0] != 'x' && sweep_axis[0] != 'y'))
                        BOOST_THROW_EXCEPTION( projection_exception(-49) );

                    if (sweep_axis[0] == 'x')
                        proj_parm.flip_axis = 1;
                    else
                        proj_parm.flip_axis = 0;
                }

                proj_parm.radius_g_1 = proj_parm.h / par.a;
                proj_parm.radius_g = 1. + proj_parm.radius_g_1;
                proj_parm.C  = proj_parm.radius_g * proj_parm.radius_g - 1.0;
                if (par.es != 0.0) {
                    proj_parm.radius_p      = sqrt (par.one_es);
                    proj_parm.radius_p2     = par.one_es;
                    proj_parm.radius_p_inv2 = par.rone_es;
                } else {
                    proj_parm.radius_p = proj_parm.radius_p2 = proj_parm.radius_p_inv2 = 1.0;
                }
            }

    }} // namespace detail::geos
    #endif // doxygen

    /*!
        \brief Geostationary Satellite View projection
        \ingroup projections
        \tparam Geographic latlong point type
        \tparam Cartesian xy point type
        \tparam Parameters parameter type
        \par Projection characteristics
         - Azimuthal
         - Spheroid
         - Ellipsoid
        \par Projection parameters
         - h: Height (real)
         - sweep: Sweep axis ('x' or 'y') (string)
        \par Example
        \image html ex_geos.gif
    */
    template <typename CalculationType, typename Parameters>
    struct geos_ellipsoid : public detail::geos::base_geos_ellipsoid<CalculationType, Parameters>
    {
        inline geos_ellipsoid(const Parameters& par) : detail::geos::base_geos_ellipsoid<CalculationType, Parameters>(par)
        {
            detail::geos::setup_geos(this->m_par, this->m_proj_parm);
        }
    };

    /*!
        \brief Geostationary Satellite View projection
        \ingroup projections
        \tparam Geographic latlong point type
        \tparam Cartesian xy point type
        \tparam Parameters parameter type
        \par Projection characteristics
         - Azimuthal
         - Spheroid
         - Ellipsoid
        \par Projection parameters
         - h: Height (real)
         - sweep: Sweep axis ('x' or 'y') (string)
        \par Example
        \image html ex_geos.gif
    */
    template <typename CalculationType, typename Parameters>
    struct geos_spheroid : public detail::geos::base_geos_spheroid<CalculationType, Parameters>
    {
        inline geos_spheroid(const Parameters& par) : detail::geos::base_geos_spheroid<CalculationType, Parameters>(par)
        {
            detail::geos::setup_geos(this->m_par, this->m_proj_parm);
        }
    };

    #ifndef DOXYGEN_NO_DETAIL
    namespace detail
    {

        // Static projection
        BOOST_GEOMETRY_PROJECTIONS_DETAIL_STATIC_PROJECTION(srs::par4::geos, geos_spheroid, geos_ellipsoid)

        // Factory entry(s)
        template <typename CalculationType, typename Parameters>
        class geos_entry : public detail::factory_entry<CalculationType, Parameters>
        {
            public :
                virtual base_v<CalculationType, Parameters>* create_new(const Parameters& par) const
                {
                    if (par.es)
                        return new base_v_fi<geos_ellipsoid<CalculationType, Parameters>, CalculationType, Parameters>(par);
                    else
                        return new base_v_fi<geos_spheroid<CalculationType, Parameters>, CalculationType, Parameters>(par);
                }
        };

        template <typename CalculationType, typename Parameters>
        inline void geos_init(detail::base_factory<CalculationType, Parameters>& factory)
        {
            factory.add_to_factory("geos", new geos_entry<CalculationType, Parameters>);
        }

    } // namespace detail
    #endif // doxygen

} // namespace projections

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_PROJECTIONS_GEOS_HPP

