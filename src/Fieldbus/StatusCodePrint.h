#pragma once

#include <MessageRouter.h>

using namespace eipScanner::cip;

const char* generalStatusCodeToString(GeneralStatusCodes code) {
    switch (code) {
        case SUCCESS:                               return "SUCCESS: Service was successfully performed.";
        case CONNECTION_FAILURE:                    return "CONNECTION_FAILURE: A connection related service failed along the connection path.";
        case RESOURCE_UNAVAILABLE:                  return "RESOURCE_UNAVAILABLE: Resources needed for the object to perform the requested service were unavailable.";
        case INVALID_PARAMETER_VALUE:               return "INVALID_PARAMETER_VALUE: See CIPStatusCodes.InvalidParameter, which is the preferred value to use for this condition.";
        case PATH_SEGMENT_ERROR:                    return "PATH_SEGMENT_ERROR: The path segment identifier or the segment syntax was not understood by the processing node. Path processing shall stop when a path segment error is encountered.";
        case PATH_DESTINATION_UNKNOWN:              return "PATH_DESTINATION_UNKNOWN: The path is referencing an object class, instance, or structure element that is not known or is not contained in the processing node. Path processing shall stop when a path destination unknown error is encountered.";
        case PARTIAL_TRANSFER:                      return "PARTIAL_TRANSFER: Only part of the expected data was transferred.";
        case CONNECTION_LOST:                       return "CONNETION_LOST: The messaging connection was lost.";
        case SERVICE_NOT_SUPPORTED:                 return "SERVICE_NOT_SUPPORTED: The requested service was not implemented or was not defined for this object Class/Instance.";
        case INVALID_ATTRIBUTE_VALUE:               return "INVALID_ATTRIBUTE_VALUE: Invalid attribute data detected.";
        case ATTRIBUTE_LIST_ERROR:                  return "ATTRIBUTE_LIST_ERROR: An attribute in the Get_Attribute_List or Set_Attribute_List response has a non-zero status.";
        case ALREADY_IN_REQUESTED_MODE_OR_STATE:    return "ALREADY_IN_REQUESTED_MODE_OR_STATE: The object is already in the mode/state being requested by the service.";
        case OBJECT_STATE_CONFLICT:                 return "OBJECT_STATE_CONFLICT: The object cannot perform the requested service in its current state/mode.";
        case OBJECT_ALREADY_EXISTS:                 return "OBJECT_ALREADY_EXISTS: The requested instance of object to be created already exists.";
        case ATTRIBUTE_NOT_SETTABLE:                return "ATTRIBUTE_NOT_SETTABLE: A request to modify a non-modifiable attribute was received.";
        case PRIVILEGE_VIOLATION:                   return "PRIVILEGE_VIOLATION: A permission/privilege check failed.";
        case DEVICE_STATE_CONFLICT:                 return "DEVICE_STATE_CONFLICT: The device's current mode/state prohibits the execution of the requested service.";
        case REPLY_DATA_TOO_LARGE:                  return "REPLY_DATA_TOO_LARGE: The data to be transmitted in the response buffer is larger than the allocated response buffer.";
        case FRAGMENTATION_OF_PRIMITIVE_VALUE:      return "FRAGMENTATION_OF_PRIMITIVE_VALUE: The service specified an operation that is going to fragment a primitive data value, i.e. half a REAL data type.";
        case NOT_ENOUGH_DATA:                       return "NOT_ENOUGH_DATA: The service did not supply enough data to perform the requested operation.";
        case ATTRIBUTE_NOT_SUPPORTED:               return "ATTRIBUTE_NOT_SUPPORTED: The attribute specified in the request is not supported.";
        case TOO_MUCH_DATA:                         return "TOO_MUCH_DATA: The service was supplied with more data than was expected.";
        case OBJECT_DOES_NOT_EXIST:                 return "OBJECT_DOES_NOT_EXIST: The object specified does not exist on the device.";
        case SVCFRAG_SEQNC_NOT_IN_PROGRESS:         return "SVCFRAG_SEQNC_NOT_IN_PROGRESS: The fragmentation sequence for this service is not currently active for this data.";
        case NO_STORED_ATTRIBUTE_DATA:              return "NO_STORED_ATTRIBUTE_DATA: The attribute data of this object was not saved prior to the requested service.";
        case STORE_OPERATION_FAILURE:               return "STORE_OPERATION_FAILURE: The attribute data of this object was not saved due to a failure following the attempt.";
        case ROUTING_FAILURE_REQUEST_SIZE:          return "ROUTING_FAILURE_REQUEST_SIZE: The service request packet was too large for transmission on a network in the path to the destination. The routing device was forced to abort the service.";
        case ROUTING_FAILURE_RESPONSE_SIZE:         return "ROUTING_FAILURE_RESPONSE_SIZE: The service response packet was too large for transmission on a network in the path from the destination. The routing device was forced to abort the service.";
        case MISSING_ATTRIBUTE_LIST_ENTRY:          return "MISSING_ATTRIBUTE_LIST_ENTRY: The service did not supply an attribute in a list of attributes that was needed by the service to perform the requested behavior.";
        case INVALID_ATTRIBUTE_LIST:                return "INVALID_ATTRIBUTE_LIST: The service is returning the list of attributes supplied with status information for those attributes that were invalid.";
        case EMBEDDED_SERVICE_ERROR:                return "EMBEDDED_SERVICE_ERROR: An embedded service resulted in an error.";
        case VENDOR_SPECIFIC:                       return "VENDOR_SPECIFIC: A vendor specific error has been encountered. The Additional Code Field of the Error Response defines the particular error encountered. Use of this General Error Code should only be performed when none of the Error Codes presented in this table or within an Object Class definition accurately reflect the error.";
        case INVALID_PARAMETER:                     return "INVALID_PARAMETER: A parameter associated with the request was invalid. This code is used when a parameter does not meet the requirements of this specification and/or the requirements defined in an Application Object Specification.";
        case WRITE_ONCE_WRITTEN:                    return "WRITE_ONCE_WRITTEN: An attempt was made to write to a write-once medium (e.g. WORM drive, PROM) that has already been written, or to modify a value that cannot be changed once established.";
        case INVALID_REPLY_RECEIVED:                return "INVALID_REPLY_RECEIVED: An invalid reply is received (e.g. reply service code does not match the request service code, or reply message is shorter than the minimum expected reply size). This status code can serve for other causes of invalid replies.";
        case KEY_FAILURE_IN_PATH:                   return "KEY_FAILURE_IN_PATH: The Key Segment that was included as the first segment in the path does not match the destination module. The object specific status shall indicate which part of the key check failed.";
        case PATH_SIZE_INVALID:                     return "PATH_SIZE_INVALID: The size of the path which was sent with the Service Request is either not large enough to allow the Request to be routed to an object or too much routing data was included.";
        case UNEXPECTED_ATTRIBUTE:                  return "UNEXPECTED_ATTRIBUTE: An attempt was made to set an attribute that is not able to be set at this time.";
        case INVALID_MEMBER_ID:                     return "INVALID_MEMBER_ID: The Member ID specified in the request does not exist in the specified Class/Instance/Attribute";
        case MEMBER_NOT_SETTABLE:                   return "MEMBER_NOT_SETTABLE: A request to modify a non-modifiable member was received";
        default:                                    return "UNKNOWN STATUS CODE";
    }
}