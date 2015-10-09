//     File: main.c
// Abstract: source code for HID LED tool, based off of HID LED test tool by
// Apple
//  Version: 1.0
//
// Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple
// Inc. ("Apple") in consideration of your agreement to the following
// terms, and your use, installation, modification or redistribution of
// this Apple software constitutes acceptance of these terms.  If you do
// not agree with these terms, please do not use, install, modify or
// redistribute this Apple software.
//
// In consideration of your agreement to abide by the following terms, and
// subject to these terms, Apple grants you a personal, non-exclusive
// license, under Apple's copyrights in this original Apple software (the
// "Apple Software"), to use, reproduce, modify and redistribute the Apple
// Software, with or without modifications, in source and/or binary forms;
// provided that if you redistribute the Apple Software in its entirety and
// without modifications, you must retain this notice and the following
// text and disclaimers in all such redistributions of the Apple Software.
// Neither the name, trademarks, service marks or logos of Apple Inc. may
// be used to endorse or promote products derived from the Apple Software
// without specific prior written permission from Apple.  Except as
// expressly stated in this notice, no other rights or licenses, express or
// implied, are granted by Apple herein, including but not limited to any
// patent rights that may be infringed by your derivative works or by other
// works in which the Apple Software may be incorporated.
//
// The Apple Software is provided by Apple on an "AS IS" basis.  APPLE
// MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION
// THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS
// FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND
// OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
//
// IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
// OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
// MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED
// AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
// STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// Copyright (C) 2015 Apple Inc. All Rights Reserved.
//
// ****************************************************
#pragma mark -
#pragma mark * complation directives *
// ----------------------------------------------------

#ifndef FALSE
#define FALSE 0
#define TRUE !FALSE
#endif

// ****************************************************
#pragma mark -
#pragma mark * includes & imports *
// ----------------------------------------------------

#include <CoreFoundation/CoreFoundation.h>
#include <Carbon/Carbon.h>
#include <IOKit/hid/IOHIDLib.h>

// ****************************************************
#pragma mark -
#pragma mark * typedef's, struct's, enums, defines, etc. *
// ----------------------------------------------------
// function to create a matching dictionary for usage page & usage
static CFMutableDictionaryRef hu_CreateMatchingDictionaryUsagePageUsage( Boolean isDeviceNotElement,
                                                                        UInt32 inUsagePage,
                                                                        UInt32 inUsage )
{
    // create a dictionary to add usage page / usages to
    CFMutableDictionaryRef result = CFDictionaryCreateMutable( kCFAllocatorDefault,
                                                              0,
                                                              &kCFTypeDictionaryKeyCallBacks,
                                                              &kCFTypeDictionaryValueCallBacks );
    
    if ( result ) {
        if ( inUsagePage ) {
            // Add key for device type to refine the matching dictionary.
            CFNumberRef pageCFNumberRef = CFNumberCreate( kCFAllocatorDefault, kCFNumberIntType, &inUsagePage );
            
            if ( pageCFNumberRef ) {
                if ( isDeviceNotElement ) {
                    CFDictionarySetValue( result, CFSTR( kIOHIDDeviceUsagePageKey ), pageCFNumberRef );
                } else {
                    CFDictionarySetValue( result, CFSTR( kIOHIDElementUsagePageKey ), pageCFNumberRef );
                }
                CFRelease( pageCFNumberRef );
                
                // note: the usage is only valid if the usage page is also defined
                if ( inUsage ) {
                    CFNumberRef usageCFNumberRef = CFNumberCreate( kCFAllocatorDefault, kCFNumberIntType, &inUsage );
                    
                    if ( usageCFNumberRef ) {
                        if ( isDeviceNotElement ) {
                            CFDictionarySetValue( result, CFSTR( kIOHIDDeviceUsageKey ), usageCFNumberRef );
                        } else {
                            CFDictionarySetValue( result, CFSTR( kIOHIDElementUsageKey ), usageCFNumberRef );
                        }
                        CFRelease( usageCFNumberRef );
                    } else {
                        fprintf( stderr, "%s: CFNumberCreate( usage ) failed.", __PRETTY_FUNCTION__ );
                    }
                }
            } else {
                fprintf( stderr, "%s: CFNumberCreate( usage page ) failed.", __PRETTY_FUNCTION__ );
            }
        }
    } else {
        fprintf( stderr, "%s: CFDictionaryCreateMutable failed.", __PRETTY_FUNCTION__ );
    }
    return result;
} // hu_CreateMatchingDictionaryUsagePageUsage

int main( int argc, const char * argv[] )
{
#pragma unused ( argc, argv )
    IOHIDDeviceRef * tIOHIDDeviceRefs = nil;
    
    // create a IO HID Manager reference
    IOHIDManagerRef tIOHIDManagerRef = IOHIDManagerCreate( kCFAllocatorDefault, kIOHIDOptionsTypeNone );
    require( tIOHIDManagerRef, Oops );
    
    // Create a device matching dictionary
    CFDictionaryRef matchingCFDictRef = hu_CreateMatchingDictionaryUsagePageUsage( TRUE,
                                                                                  kHIDPage_GenericDesktop,
                                                                                  kHIDUsage_GD_Keyboard );
    require( matchingCFDictRef, Oops );
    
    // set the HID device matching dictionary
    IOHIDManagerSetDeviceMatching( tIOHIDManagerRef, matchingCFDictRef );
    
    if ( matchingCFDictRef ) {
        CFRelease( matchingCFDictRef );
    }
    
    // Now open the IO HID Manager reference
    IOReturn tIOReturn = IOHIDManagerOpen( tIOHIDManagerRef, kIOHIDOptionsTypeNone );
    require_noerr( tIOReturn, Oops );
    
    // and copy out its devices
    CFSetRef deviceCFSetRef = IOHIDManagerCopyDevices( tIOHIDManagerRef );
    require( deviceCFSetRef, Oops );
    
    // how many devices in the set?
    CFIndex deviceIndex, deviceCount = CFSetGetCount( deviceCFSetRef );
    
    // allocate a block of memory to extact the device ref's from the set into
    tIOHIDDeviceRefs = malloc( sizeof( IOHIDDeviceRef ) * deviceCount );
    if (!tIOHIDDeviceRefs) {
        CFRelease(deviceCFSetRef);
        deviceCFSetRef = NULL;
        goto Oops;
    }
    
    // now extract the device ref's from the set
    CFSetGetValues( deviceCFSetRef, (const void **) tIOHIDDeviceRefs );
    CFRelease(deviceCFSetRef);
    deviceCFSetRef = NULL;
    
    // before we get into the device loop we'll setup our element matching dictionary
    matchingCFDictRef = hu_CreateMatchingDictionaryUsagePageUsage( FALSE, kHIDPage_LEDs, 0 );
    require( matchingCFDictRef, Oops );
    
    for ( deviceIndex = 0; deviceIndex < deviceCount; deviceIndex++ ) {
        
        // if this isn't a keyboard device...
        if ( !IOHIDDeviceConformsTo( tIOHIDDeviceRefs[deviceIndex], kHIDPage_GenericDesktop, kHIDUsage_GD_Keyboard ) ) {
            continue; // ...skip it
        }
        
        // copy all the elements
        CFArrayRef elementCFArrayRef = IOHIDDeviceCopyMatchingElements( tIOHIDDeviceRefs[deviceIndex],
                                                                       matchingCFDictRef,
                                                                       kIOHIDOptionsTypeNone );
        require( elementCFArrayRef, next_device );
        
        int device_value;
        if (argc == 1) {
            device_value = 0xFF;
        } else {
            device_value = 0x00;
        }
        
        // iterate over all the elements
        CFIndex elementIndex, elementCount = CFArrayGetCount( elementCFArrayRef );
        for ( elementIndex = 0; elementIndex < elementCount; elementIndex++ ) {
            IOHIDElementRef tIOHIDElementRef = ( IOHIDElementRef ) CFArrayGetValueAtIndex( elementCFArrayRef, elementIndex );
            require( tIOHIDElementRef, next_element );
            
            uint32_t usagePage = IOHIDElementGetUsagePage( tIOHIDElementRef );
            
            // if this isn't an LED element...
            if ( kHIDPage_LEDs != usagePage ) {
                continue; // ...skip it
            }
            
            // get the logical mix/max for this LED element
            CFIndex minCFIndex = IOHIDElementGetLogicalMin( tIOHIDElementRef );
            CFIndex maxCFIndex = IOHIDElementGetLogicalMax( tIOHIDElementRef );
            
            // calculate the range
            CFIndex modCFIndex = maxCFIndex - minCFIndex + 1;
            
            // compute the value for this LED element
            CFIndex tCFIndex = minCFIndex + ( device_value % modCFIndex );
            device_value /= modCFIndex;
            
            //  printf( "      value = 0x%08lX.\n", tCFIndex );
            
            uint64_t timestamp = 0; // create the IO HID Value to be sent to this LED element
            IOHIDValueRef tIOHIDValueRef = IOHIDValueCreateWithIntegerValue( kCFAllocatorDefault, tIOHIDElementRef, timestamp, tCFIndex );
            if ( tIOHIDValueRef ) {
                // now set it on the device
                tIOReturn = IOHIDDeviceSetValue( tIOHIDDeviceRefs[deviceIndex], tIOHIDElementRef, tIOHIDValueRef );
                CFRelease( tIOHIDValueRef );
               // require_noerr( tIOReturn, next_element );
            }
        next_element: ;
            continue;
        }
        CFRelease( elementCFArrayRef );
    next_device: ;
        continue;
    }
    
    if ( matchingCFDictRef ) {
        CFRelease( matchingCFDictRef );
    }
Oops: ;
    if ( tIOHIDDeviceRefs ) {
        free(tIOHIDDeviceRefs);
    }
    
    if ( tIOHIDManagerRef ) {
        CFRelease( tIOHIDManagerRef );
    }
    return 0;
} /* main */