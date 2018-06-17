//
//  gash_swift_api.hpp
//  CircuitExecutor
//
//  Created by Xiaoting Tang on 6/16/18.
//  Copyright © 2018 Xiaoting Tang. All rights reserved.
//

#ifndef gash_swift_api_hpp
#define gash_swift_api_hpp

typedef struct Circuit CCircuit;
typedef struct DivCircuit CDivCircuit;
typedef struct GarbledCircuit CGarbledCircuit;

#ifdef __cplusplus
extern "C" {
#endif
    
    CCircuit* CreateCircuit();
    void ExecuteCircuit(CCircuit*);
    const char* GetOutputString(CCircuit*);
    CDivCircuit* CreateCDivCircuit(int bitsize, int denom, int nume);
    void BuildDivCircuit(CDivCircuit*);
    
    
#ifdef __cplusplus
}
#endif


#endif /* gash_swift_api_hpp */
