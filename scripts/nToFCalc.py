#!@Python3_EXECUTABLE@

import argparse
import math
from scipy.integrate import quad

# Physical constants
C = 299792458.0                  # Speed of light (m/s)
NEUTRON_REST_ENERGY = 939.56542194  # MeV
NEUTRON_REST_ENERGY_UNCERTAINTY = 0.00000000048


def radial_distance_rms(d, l, r):
    """
    Calculate the RMS additional flight distance caused by radial
    displacement of the neutron interaction point within a cylinder.

    Assumptions:
      - Point source on the cylinder axis.
      - Uniform interaction probability throughout the cylinder.
      - z is uniform from 0 to l.
      - Radial position follows the uniform-area distribution.

    Returns:
      RMS additional radial flight distance in meters.
    """

    if r == 0:
        return 0.0

    def radial_mean_square(z):
        x = d + z

        def integrand(rho):
            # Additional path length due to radial displacement.
            # Rationalized form avoids numerical cancellation.
            delta = rho**2 / (
                math.sqrt(x**2 + rho**2) + x
            )

            # Radial probability density: 2*rho/r**2
            return delta**2 * (2.0 * rho / r**2)

        result, _ = quad(integrand, 0.0, r)
        return result

    # Average the radial mean-square contribution over detector length.
    mean_square, _ = quad(radial_mean_square, 0.0, l)
    mean_square /= l

    return math.sqrt(max(0.0, mean_square))


def geometry_uncertainty(d, l, r):
    """
    Combine the existing axial uncertainty (l/2) with the additional
    RMS radial flight-distance contribution.
    """

    axial_error = l / 2.0
    radial_error = radial_distance_rms(d, l, r)

    total_error = math.sqrt(
        axial_error**2 + radial_error**2
    )

    return axial_error, radial_error, total_error


def neutron_energy(tof_ns, d, l, r, dt_ns):
    """
    Calculate relativistic neutron kinetic energy and uncertainty.

    The centroid energy uses only nominal distance d and TOF.
    Detector geometry contributes only to the uncertainty.
    """

    if tof_ns <= 0:
        raise ValueError("TOF must be positive.")

    if d <= 0:
        raise ValueError("Flight distance must be positive.")

    if l <= 0:
        raise ValueError("Detector length must be positive.")

    if r < 0:
        raise ValueError("Detector radius cannot be negative.")

    if dt_ns < 0:
        raise ValueError("Timing uncertainty cannot be negative.")

    # Convert TOF to seconds.
    tof = tof_ns * 1e-9
    dt = dt_ns * 1e-9

    # Centroid velocity: nominal distance only.
    beta = d / (C * tof)

    if beta >= 1.0:
        raise ValueError(
            "Calculated neutron velocity is greater than or equal to c."
        )

    gamma = 1.0 / math.sqrt(1.0 - beta**2)

    # Relativistic kinetic energy (MeV).
    energy = (gamma - 1.0) * NEUTRON_REST_ENERGY

    # Derivative of kinetic energy with respect to distance.
    dE_dd = (
        NEUTRON_REST_ENERGY
        * gamma**3
        * beta**2
        / d
    )

    # Derivative magnitude with respect to time.
    dE_dt = (
        NEUTRON_REST_ENERGY
        * gamma**3
        * beta**2
        / tof
    )

    # Geometry uncertainty.
    axial_error, radial_error, distance_error = (
        geometry_uncertainty(d, l, r)
    )

    energy_error_geometry = dE_dd * distance_error

    # Timing uncertainty.
    energy_error_timing = dE_dt * dt

    # Neutron rest-energy uncertainty.
    energy_error_mass = (
        energy
        * NEUTRON_REST_ENERGY_UNCERTAINTY
        / NEUTRON_REST_ENERGY
    )

    # Total uncertainty.
    energy_error_total = math.sqrt(
        energy_error_geometry**2
        + energy_error_timing**2
        + energy_error_mass**2
    )

    return {
        "energy": energy,
        "total_error": energy_error_total,
        "geometry_error": energy_error_geometry,
        "timing_error": energy_error_timing,
        "mass_error": energy_error_mass,
        "axial_distance_error": axial_error,
        "radial_distance_error": radial_error,
        "total_distance_error": distance_error,
        "beta": beta,
        "gamma": gamma,
    }


def main():
    parser = argparse.ArgumentParser(
        description="Calculate relativistic neutron TOF energy."
    )

    parser.add_argument(
        "-t", "--tof",
        type=float,
        required=True,
        help="Neutron time of flight (ns)"
    )

    parser.add_argument(
        "-d", "--distance",
        type=float,
        required=True,
        help="Nominal source-to-detector distance (m)"
    )

    parser.add_argument(
        "-l", "--length",
        type=float,
        required=True,
        help="Detector cylinder length (m)"
    )

    parser.add_argument(
        "-r", "--radius",
        type=float,
        required=True,
        help="Detector cylinder radius (m)"
    )

    parser.add_argument(
        "--dt",
        type=float,
        default=0.0,
        help="TOF timing uncertainty (ns)"
    )

    args = parser.parse_args()

    try:
        result = neutron_energy(
            args.tof,
            args.distance,
            args.length,
            args.radius,
            args.dt
        )
    except ValueError as error:
        parser.error(str(error))


    print(
        f"Result: {result['energy'] * 1000:.6f} "
        f"+/- {result['total_error'] * 1000:.6f} keV"
    )

    print(f"Neutron kinetic energy: "
          f"{result['energy']:.6f} MeV")

    print(f"Total energy uncertainty: "
          f"{result['total_error']:.6f} MeV")

    print("\nUncertainty contributions:")

    print(f"  Axial distance uncertainty: "
          f"{result['axial_distance_error']:.6e} m")

    print(f"  Radial distance uncertainty: "
          f"{result['radial_distance_error']:.6e} m")

    print(f"  Combined distance uncertainty: "
          f"{result['total_distance_error']:.6e} m")

    print(f"  Geometry: "
          f"{result['geometry_error']:.6f} MeV")

    print(f"  Timing: "
          f"{result['timing_error']:.6f} MeV")

    print(f"  Neutron mass: "
          f"{result['mass_error']:.6e} MeV")

    print(f"\nBeta:  {result['beta']:.8f}")
    print(f"Gamma: {result['gamma']:.8f}")


if __name__ == "__main__":
    main()

