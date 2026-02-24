#!/usr/bin/env python3
import os
import sys
import subprocess
import shutil
import argparse

def run_command(command, cwd=None):
    print(f"Executing: {' '.join(command)}")
    try:
        subprocess.check_call(command, cwd=cwd)
        return True
    except subprocess.CalledProcessError as e:
        ret = e.returncode
        # On Unix, negative return code means killed by signal
        # Our CrashHandler uses _exit(signal), so positive 11, 6, etc. are also crashes
        crash_codes = [6, 11, 4, 8, 10] # ABRT, SEGV, ILL, FPE, BUS
        if ret < 0 or ret in crash_codes:
            print(f"\n[!] ENGINE CRASH DETECTED (Exit Code: {ret})")
            print("[!] Please check the 'crashes/' folder for a diagnostic report.")
        else:
            print(f"Error: Command failed with exit code {ret}")
        return False

def main():
    parser = argparse.ArgumentParser(description="Spark Engine Build & Run Script")
    parser.add_argument("-r", "--run", action="store_true", help="Run the engine after building")
    parser.add_argument("-c", "--clean", action="store_true", help="Clean build directory before building")
    parser.add_argument("-j", "--jobs", type=int, default=4, help="Number of parallel make jobs")
    args = parser.parse_args()

    project_root = os.path.dirname(os.path.abspath(__file__))
    build_dir = os.path.join(project_root, "build")

    # 1. Clean if requested
    if args.clean and os.path.exists(build_dir):
        print("Cleaning build directory...")
        shutil.rmtree(build_dir)

    # 2. Ensure build directory exists
    if not os.path.exists(build_dir):
        os.makedirs(build_dir)

    # 3. CMake Configuration
    # Use absolute path to project root for stability
    if not run_command(["cmake", project_root], cwd=build_dir):
        sys.exit(1)

    # 4. Build
    if not run_command(["make", f"-j{args.jobs}"], cwd=build_dir):
        sys.exit(1)

    print("\nBuild successful!")

    # 5. Run
    if args.run:
        print("\nStarting Spark Engine...")
        executable = os.path.join(build_dir, "Spark")
        if not os.path.exists(executable):
            print(f"Error: Executable not found at {executable}")
            sys.exit(1)
            
        # Start from project root so relative paths like 'assets/' work correctly
        run_command([executable], cwd=project_root)

if __name__ == "__main__":
    main()
