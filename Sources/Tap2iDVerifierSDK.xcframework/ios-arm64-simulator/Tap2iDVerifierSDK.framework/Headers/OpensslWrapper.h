//
//  OpensslWrapper.h
//  MIDReader
//
//  Created by Vijay Radake on 06/01/23.
//

#ifndef OpensslWrapper_h
#define OpensslWrapper_h

#import <Foundation/Foundation.h>
@interface OpensslWrapper: NSObject

- (NSString *)createX509Certificate:(int)serialNumber subjectDN:(NSString *)subjectDN issuerDN:(NSString*)issuerDN publicKey:(NSData *)publicKey privateKeynew:(NSData *)privateKeynew startDate:(long)startDate endDate:(long)endDate extExtendedKeyUsage:(NSString*)extExtendedKeyUsage extAuthorityKey:(NSString*)extAuthorityKey certificateData:(NSData *)certificateData cert_size:(int)cert_size;
@end

#endif /* OpensslWrapper_h */
