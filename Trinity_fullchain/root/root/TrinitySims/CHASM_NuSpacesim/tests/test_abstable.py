#!/usr/bin/env python3
"""
Test script to verify the abstable switching functionality
"""

import numpy as np
import sys
import os

# Add the src directory to Python path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), 'src'))

try:
    import CHASM as ch
    print("✓ CHASM module imported successfully")
except ImportError as e:
    print(f"✗ Failed to import CHASM: {e}")
    sys.exit(1)

def test_abstable_switching():
    """Test the abstable switching functionality"""
    print("\n=== Testing Abstable Switching ===")
    
    try:
        # Test switching to 'original'
        print("Setting abstable to 'corsika'...")
        ch.useAbstable('corsika')
        print("✓ Successfully set to 'corsika'")
        
        # Test switching to 'm5'
        print("Setting abstable to 'm5'...")
        ch.useAbstable('m5')
        print("✓ Successfully set to 'm5'")

        # test switching to 'm5_new'
        print("Setting abstable to 'm5_new'...")
        ch.useAbstable('m5_new')
        print("✓ Successfully set to 'm5_new'")
        
        # Test accessing the data
        from CHASM.axis import _abstable_manager
        data = _abstable_manager.get_current_data()
        print(f"✓ Current abstable data loaded: {list(data.keys())}")
        
        # Test invalid abstable name
        print("Testing invalid abstable name...")
        try:
            ch.useAbstable('nonexistent')
            print("✗ Should have raised an error for invalid abstable")
        except ValueError as e:
            print(f"✓ Correctly raised error: {e}")
        
        return True
        
    except Exception as e:
        print(f"✗ Error during abstable testing: {e}")
        return False

def test_attenuation_class():
    """Test that the Attenuation class can access abstable data"""
    print("\n=== Testing Attenuation Class ===")
    
    try:
        # Set a specific abstable
        ch.useAbstable('m5_new')
        
        # Try to access attenuation properties via the manager
        from CHASM.axis import _abstable_manager
        
        # Check if the required properties are accessible through the manager
        data_mgr = _abstable_manager
        if hasattr(data_mgr, 'ecoeff') and hasattr(data_mgr, 'l_list') and hasattr(data_mgr, 'altitude_list'):
            print("✓ All required abstable data properties accessible")
            print(f"  - ecoeff shape: {data_mgr.ecoeff.shape}")
            print(f"  - l_list shape: {data_mgr.l_list.shape}")
            print(f"  - altitude_list shape: {data_mgr.altitude_list.shape}")
        else:
            print(f"✗ Missing properties in abstable manager")
            return False
        
        # Also verify raw data has correct keys
        raw_data = data_mgr.get_current_data()
        expected_keys = ['ecoeff', 'wavelength', 'height']
        if all(key in raw_data for key in expected_keys):
            print("✓ Raw NPZ data contains expected keys:", list(raw_data.keys()))
        else:
            print(f"✗ Missing keys in raw data. Expected {expected_keys}, got {list(raw_data.keys())}")
            return False
            
        return True
        
    except Exception as e:
        print(f"✗ Error during attenuation testing: {e}")
        return False

if __name__ == "__main__":
    print("Starting abstable functionality tests...")
    
    test1_passed = test_abstable_switching()
    test2_passed = test_attenuation_class()
    
    if test1_passed and test2_passed:
        print("\n🎉 All tests passed! The abstable switching functionality is working correctly.")
    else:
        print("\n❌ Some tests failed. Please check the implementation.")
        sys.exit(1)